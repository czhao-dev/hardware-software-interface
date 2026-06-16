`timescale 1ns / 1ps

// Pipeline-aware integration testbench for the 2-stage IF/EX-WB CPU.
//
// Timing relative to Phase 1 (single-cycle):
//   - One extra "fill" step at the start empties the reset NOP from
//     the pipeline register; every subsequent step executes exactly
//     one real instruction in EX-WB.
//   - Each taken branch or jump inserts one bubble step: the pipeline
//     register holds NOP (0xF000) for that cycle and no architectural
//     state changes.  These bubble steps are verified explicitly.
//   - The IF-stage PC (top-level `pc` output) is one ahead of EX-WB,
//     so checked values are consistently +1 compared to Phase 1.
//   - After HALT the IF-stage PC is frozen at its current value (18),
//     one beyond the HALT instruction address (17).
//
// Test program layout (same instructions as Phase 1):
//   addr  0: LDI  R0, 4
//   addr  1: LDI  R1, 5
//   addr  2: ADD  R2, R0, R1
//   addr  3: SUB  R3, R0, R1
//   addr  4: ANDI R2, 8
//   addr  5: LDI  R4, 1
//   addr  6: SLL  R2, R2, R4
//   addr  7: SLT  R3, R0, R1
//   addr  8: ST   R3, 5
//   addr  9: LD   R1, 5
//   addr 10: SUB  R2, R1, R1       ; sets Z=1
//   addr 11: BNE  +3               ; not taken
//   addr 12: BEQ  +3               ; taken -> pc=15  [bubble]
//   addr 13: LDI  R0, 0xFF         ; trap (skipped)
//   addr 14: LDI  R0, 0xFF         ; trap (skipped)
//   addr 15: JAL  18               ; R7=16, pc=18    [bubble]
//   addr 16: LDI  R6, 0x99         ; return target
//   addr 17: HALT
//   addr 18: LDI  R5, 0x2A         ; subroutine body
//   addr 19: RET                   ; pc=R7=16        [bubble]

module top_tb;
    reg clk;
    reg reset;
    reg run_enable;
    reg reg_write_enable;
    reg [2:0] reg_write_addr;
    reg [7:0] reg_write_data;
    reg instr_write_enable;
    reg [7:0] instr_write_addr;
    reg [15:0] instr_write_data;
    reg dmem_write_enable;
    reg [7:0] dmem_write_addr;
    reg [7:0] dmem_write_data;

    wire [7:0] pc;
    wire [15:0] instruction;
    wire [7:0] result;
    wire zero;
    wire negative;
    wire carry;
    wire overflow;
    wire halted;

    integer failures;

    top uut (
        .clk(clk),
        .reset(reset),
        .run_enable(run_enable),
        .reg_write_enable(reg_write_enable),
        .reg_write_addr(reg_write_addr),
        .reg_write_data(reg_write_data),
        .instr_write_enable(instr_write_enable),
        .instr_write_addr(instr_write_addr),
        .instr_write_data(instr_write_data),
        .dmem_write_enable(dmem_write_enable),
        .dmem_write_addr(dmem_write_addr),
        .dmem_write_data(dmem_write_data),
        .pc(pc),
        .instruction(instruction),
        .result(result),
        .zero(zero),
        .negative(negative),
        .carry(carry),
        .overflow(overflow),
        .halted(halted)
    );

    always #5 clk = ~clk;

    // -----------------------------------------------------------------
    // ISA v2 encode helpers (identical to the assembler's encodings)
    // -----------------------------------------------------------------

    function [15:0] encode_alu_reg;
        input [3:0] op;
        input [2:0] dest;
        input [2:0] src_a;
        input [2:0] src_b;
        begin
            encode_alu_reg = {2'b00, op, dest, src_a, src_b, 1'b0};
        end
    endfunction

    function [15:0] encode_alu_imm;
        input [2:0] subop;
        input [2:0] dest;
        input [7:0] imm;
        begin
            encode_alu_imm = {2'b01, subop, dest, imm};
        end
    endfunction

    function [15:0] encode_branch;
        input [2:0] cond;
        input [7:0] offset;
        begin
            encode_branch = {2'b10, cond, offset, 3'b000};
        end
    endfunction

    function [15:0] encode_jump;
        input [7:0] target;
        input link;
        begin
            encode_jump = {2'b11, 2'b00, target, link, 3'b000};
        end
    endfunction

    function [15:0] encode_load;
        input [2:0] dest;
        input [7:0] addr;
        begin
            encode_load = {2'b11, 2'b01, dest, addr, 1'b0};
        end
    endfunction

    function [15:0] encode_store;
        input [2:0] src;
        input [7:0] addr;
        begin
            encode_store = {2'b11, 2'b10, src, addr, 1'b0};
        end
    endfunction

    function [15:0] encode_system;
        input [1:0] sysop;
        begin
            encode_system = {2'b11, 2'b11, sysop, 10'b0};
        end
    endfunction

    localparam [3:0] OP_ADD = 4'b0000;
    localparam [3:0] OP_SUB = 4'b0001;
    localparam [3:0] OP_SLL = 4'b0101;
    localparam [3:0] OP_SLT = 4'b0111;
    localparam [2:0] SUB_LDI  = 3'b000;
    localparam [2:0] SUB_ANDI = 3'b011;
    localparam [2:0] COND_BEQ = 3'b000;
    localparam [2:0] COND_BNE = 3'b001;
    localparam [1:0] SYS_HALT = 2'b01;
    localparam [1:0] SYS_RET  = 2'b10;
    localparam [15:0] NOP_WORD = 16'hF000;

    // -----------------------------------------------------------------
    // Helper tasks
    // -----------------------------------------------------------------

    task expect_equal;
        input [127:0] label;
        input [7:0] actual;
        input [7:0] expected;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: expected %h, got %h", label, expected, actual);
                failures = failures + 1;
            end
        end
    endtask

    task expect_flag;
        input [127:0] label;
        input actual;
        input expected;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: expected %b, got %b", label, expected, actual);
                failures = failures + 1;
            end
        end
    endtask

    task write_instruction;
        input [7:0] addr;
        input [15:0] data;
        begin
            @(negedge clk);
            instr_write_enable = 1'b1;
            instr_write_addr = addr;
            instr_write_data = data;
            @(negedge clk);
            instr_write_enable = 1'b0;
        end
    endtask

    // Advance the pipeline by one clock cycle.
    task step;
        begin
            @(negedge clk);
            run_enable = 1'b1;
            @(posedge clk);
            #1;
            @(negedge clk);
            run_enable = 1'b0;
        end
    endtask

    initial begin
        clk = 1'b0;
        failures = 0;
        reset = 1'b1;
        run_enable = 1'b0;
        reg_write_enable = 1'b0;
        reg_write_addr = 3'b000;
        reg_write_data = 8'h00;
        instr_write_enable = 1'b0;
        instr_write_addr = 8'h00;
        instr_write_data = 16'h0000;
        dmem_write_enable = 1'b0;
        dmem_write_addr = 8'h00;
        dmem_write_data = 8'h00;

        $dumpfile("waveform.vcd");
        $dumpvars(0, top_tb);

        #12 reset = 1'b0;

        write_instruction(8'd0,  encode_alu_imm(SUB_LDI, 3'd0, 8'h04));
        write_instruction(8'd1,  encode_alu_imm(SUB_LDI, 3'd1, 8'h05));
        write_instruction(8'd2,  encode_alu_reg(OP_ADD, 3'd2, 3'd0, 3'd1));
        write_instruction(8'd3,  encode_alu_reg(OP_SUB, 3'd3, 3'd0, 3'd1));
        write_instruction(8'd4,  encode_alu_imm(SUB_ANDI, 3'd2, 8'h08));
        write_instruction(8'd5,  encode_alu_imm(SUB_LDI, 3'd4, 8'h01));
        write_instruction(8'd6,  encode_alu_reg(OP_SLL, 3'd2, 3'd2, 3'd4));
        write_instruction(8'd7,  encode_alu_reg(OP_SLT, 3'd3, 3'd0, 3'd1));
        write_instruction(8'd8,  encode_store(3'd3, 8'd5));
        write_instruction(8'd9,  encode_load(3'd1, 8'd5));
        write_instruction(8'd10, encode_alu_reg(OP_SUB, 3'd2, 3'd1, 3'd1));
        write_instruction(8'd11, encode_branch(COND_BNE, 8'd3));
        write_instruction(8'd12, encode_branch(COND_BEQ, 8'd3));
        write_instruction(8'd13, encode_alu_imm(SUB_LDI, 3'd0, 8'hFF));
        write_instruction(8'd14, encode_alu_imm(SUB_LDI, 3'd0, 8'hFF));
        write_instruction(8'd15, encode_jump(8'd18, 1'b1));
        write_instruction(8'd16, encode_alu_imm(SUB_LDI, 3'd6, 8'h99));
        write_instruction(8'd17, encode_system(SYS_HALT));
        write_instruction(8'd18, encode_alu_imm(SUB_LDI, 3'd5, 8'h2A));
        write_instruction(8'd19, encode_system(SYS_RET));

        // -----------------------------------------------------------------
        // Fill step: EX-WB drains the reset NOP; addr 0 enters EX-WB next.
        // -----------------------------------------------------------------
        step;
        expect_equal("pc after fill", pc, 8'd1);

        // -----------------------------------------------------------------
        // addr 0: LDI R0, 4
        // -----------------------------------------------------------------
        step;
        expect_equal("R0 after LDI 4", uut.rf.registers[0], 8'h04);
        expect_equal("pc after LDI R0", pc, 8'd2);
        expect_flag("fz after LDI R0", uut.flag_zero, 1'b0);
        expect_flag("fn after LDI R0", uut.flag_negative, 1'b0);
        expect_flag("fc after LDI R0", uut.flag_carry, 1'b0);
        expect_flag("fo after LDI R0", uut.flag_overflow, 1'b0);

        // -----------------------------------------------------------------
        // addr 1: LDI R1, 5
        // -----------------------------------------------------------------
        step;
        expect_equal("R1 after LDI 5", uut.rf.registers[1], 8'h05);
        expect_equal("pc after LDI R1", pc, 8'd3);

        // -----------------------------------------------------------------
        // addr 2: ADD R2, R0, R1  ->  R2 = 9
        // Check combinational EX-WB outputs before the clock edge.
        // -----------------------------------------------------------------
        @(negedge clk);
        run_enable = 1'b1;
        #1;
        expect_equal("result ADD", result, 8'h09);
        expect_flag("zero ADD", zero, 1'b0);
        expect_flag("negative ADD", negative, 1'b0);
        expect_flag("carry ADD", carry, 1'b0);
        expect_flag("overflow ADD", overflow, 1'b0);
        @(posedge clk);
        #1;
        @(negedge clk);
        run_enable = 1'b0;
        expect_equal("R2 after ADD", uut.rf.registers[2], 8'h09);
        expect_equal("pc after ADD", pc, 8'd4);

        // -----------------------------------------------------------------
        // addr 3: SUB R3, R0, R1  ->  R3 = 0xFF, N=1, C=1
        // -----------------------------------------------------------------
        @(negedge clk);
        run_enable = 1'b1;
        #1;
        expect_equal("result SUB", result, 8'hFF);
        expect_flag("zero SUB", zero, 1'b0);
        expect_flag("negative SUB", negative, 1'b1);
        expect_flag("carry SUB", carry, 1'b1);
        expect_flag("overflow SUB", overflow, 1'b0);
        @(posedge clk);
        #1;
        @(negedge clk);
        run_enable = 1'b0;
        expect_equal("R3 after SUB", uut.rf.registers[3], 8'hFF);
        expect_equal("pc after SUB", pc, 8'd5);

        // -----------------------------------------------------------------
        // addr 4: ANDI R2, 8  ->  R2 = 8
        // -----------------------------------------------------------------
        step;
        expect_equal("R2 after ANDI", uut.rf.registers[2], 8'h08);
        expect_equal("pc after ANDI", pc, 8'd6);
        expect_flag("fz after ANDI", uut.flag_zero, 1'b0);
        expect_flag("fn after ANDI", uut.flag_negative, 1'b0);
        expect_flag("fc after ANDI", uut.flag_carry, 1'b0);
        expect_flag("fo after ANDI", uut.flag_overflow, 1'b0);

        // -----------------------------------------------------------------
        // addr 5: LDI R4, 1
        // -----------------------------------------------------------------
        step;
        expect_equal("R4 after LDI 1", uut.rf.registers[4], 8'h01);
        expect_equal("pc after LDI R4", pc, 8'd7);

        // -----------------------------------------------------------------
        // addr 6: SLL R2, R2, R4  ->  R2 = 16
        // -----------------------------------------------------------------
        step;
        expect_equal("R2 after SLL", uut.rf.registers[2], 8'h10);
        expect_equal("pc after SLL", pc, 8'd8);
        expect_flag("fc after SLL", uut.flag_carry, 1'b0);

        // -----------------------------------------------------------------
        // addr 7: SLT R3, R0, R1  ->  R3 = 1
        // -----------------------------------------------------------------
        step;
        expect_equal("R3 after SLT", uut.rf.registers[3], 8'h01);
        expect_equal("pc after SLT", pc, 8'd9);

        // -----------------------------------------------------------------
        // addr 8: ST R3, 5  ->  dmem[5] = 1
        // -----------------------------------------------------------------
        step;
        expect_equal("dmem[5] after ST", uut.dmem.memory[5], 8'h01);
        expect_equal("pc after ST", pc, 8'd10);

        // -----------------------------------------------------------------
        // addr 9: LD R1, 5  ->  R1 = 1
        // -----------------------------------------------------------------
        step;
        expect_equal("R1 after LD", uut.rf.registers[1], 8'h01);
        expect_equal("pc after LD", pc, 8'd11);

        // -----------------------------------------------------------------
        // addr 10: SUB R2, R1, R1  ->  R2 = 0, Z=1
        // -----------------------------------------------------------------
        step;
        expect_equal("R2 after SUB R1 R1", uut.rf.registers[2], 8'h00);
        expect_equal("pc after SUB R1 R1", pc, 8'd12);
        expect_flag("fz after SUB R1 R1", uut.flag_zero, 1'b1);
        expect_flag("fn after SUB R1 R1", uut.flag_negative, 1'b0);
        expect_flag("fc after SUB R1 R1", uut.flag_carry, 1'b0);
        expect_flag("fo after SUB R1 R1", uut.flag_overflow, 1'b0);

        // -----------------------------------------------------------------
        // addr 11: BNE +3  ->  not taken (Z=1), pc = 13
        // -----------------------------------------------------------------
        step;
        expect_equal("pc after BNE not-taken", pc, 8'd13);

        // -----------------------------------------------------------------
        // addr 12: BEQ +3  ->  taken (Z=1), pc = 15; bubble injected.
        //   instr[13] (trap) was speculatively fetched but is squashed.
        // -----------------------------------------------------------------
        step;
        expect_equal("pc after BEQ taken", pc, 8'd15);

        // Bubble: pipeline register holds NOP, no architectural side effects.
        @(negedge clk);
        run_enable = 1'b1;
        #1;
        expect_equal("bubble BEQ: NOP in EX-WB", instruction, NOP_WORD);
        @(posedge clk);
        #1;
        @(negedge clk);
        run_enable = 1'b0;
        expect_equal("pc after BEQ bubble", pc, 8'd16);

        // -----------------------------------------------------------------
        // addr 15: JAL 18  ->  R7 = ex_pc+1 = 16, pc = 18; bubble injected.
        // -----------------------------------------------------------------
        step;
        expect_equal("R7 after JAL", uut.rf.registers[7], 8'd16);
        expect_equal("pc after JAL", pc, 8'd18);

        // Bubble after JAL.
        @(negedge clk);
        run_enable = 1'b1;
        #1;
        expect_equal("bubble JAL: NOP in EX-WB", instruction, NOP_WORD);
        @(posedge clk);
        #1;
        @(negedge clk);
        run_enable = 1'b0;
        expect_equal("pc after JAL bubble", pc, 8'd19);

        // -----------------------------------------------------------------
        // addr 18: LDI R5, 0x2A  (subroutine body)
        // -----------------------------------------------------------------
        step;
        expect_equal("R5 after LDI 0x2A", uut.rf.registers[5], 8'h2A);
        expect_equal("pc after LDI R5", pc, 8'd20);

        // -----------------------------------------------------------------
        // addr 19: RET  ->  pc = R7 = 16; bubble injected.
        // -----------------------------------------------------------------
        step;
        expect_equal("pc after RET", pc, 8'd16);

        // Bubble after RET.
        @(negedge clk);
        run_enable = 1'b1;
        #1;
        expect_equal("bubble RET: NOP in EX-WB", instruction, NOP_WORD);
        @(posedge clk);
        #1;
        @(negedge clk);
        run_enable = 1'b0;
        expect_equal("pc after RET bubble", pc, 8'd17);

        // -----------------------------------------------------------------
        // addr 16: LDI R6, 0x99  (execution after RET)
        // -----------------------------------------------------------------
        step;
        expect_equal("R6 after LDI 0x99", uut.rf.registers[6], 8'h99);
        expect_equal("pc after LDI R6", pc, 8'd18);

        // -----------------------------------------------------------------
        // addr 17: HALT  ->  halted = 1, IF-stage pc frozen at 18
        // -----------------------------------------------------------------
        step;
        expect_flag("halted after HALT", halted, 1'b1);
        expect_equal("pc frozen after HALT", pc, 8'd18);

        // Extra step while halted: nothing should change.
        step;
        expect_flag("halted stays set", halted, 1'b1);
        expect_equal("pc unchanged after extra step", pc, 8'd18);

        // R0 must still be 4: the trap instructions at addrs 13/14 were
        // squashed by the BEQ flush and never reached EX-WB.
        expect_equal("R0 unchanged (trap skipped)", uut.rf.registers[0], 8'h04);

        if (failures == 0) begin
            $display("top_tb: PASS");
        end else begin
            $display("top_tb: FAIL (%0d failures)", failures);
            $finish_and_return(1);
        end

        $finish;
    end
endmodule
