`timescale 1ns / 1ps

module top_tb;
    reg clk;
    reg reset;
    reg run_enable;
    reg reg_write_enable;
    reg [1:0] reg_write_addr;
    reg [3:0] reg_write_data;
    reg instr_write_enable;
    reg [3:0] instr_write_addr;
    reg [15:0] instr_write_data;
    reg dmem_write_enable;
    reg [3:0] dmem_write_addr;
    reg [3:0] dmem_write_data;
    wire [3:0] pc;
    wire [15:0] instruction;
    wire [3:0] result;
    wire zero;
    wire negative;
    wire carry;
    wire overflow;
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
        .overflow(overflow)
    );

    always #5 clk = ~clk;

    // Class 00: ALU register-register / register-immediate op
    function [15:0] encode_alu;
        input [2:0] opcode;
        input [1:0] dest;
        input [1:0] src_a;
        input [1:0] src_b;
        input use_immediate;
        input [3:0] immediate;
        begin
            encode_alu = {2'b00, opcode, dest, src_a, src_b, use_immediate, immediate};
        end
    endfunction

    // Class 01: conditional branch, pc <= pc + offset when condition is met
    function [15:0] encode_branch;
        input [2:0] cond;
        input [3:0] offset;
        begin
            encode_branch = {2'b01, cond, 7'b0000000, offset};
        end
    endfunction

    // Class 10: unconditional jump, pc <= target
    function [15:0] encode_jump;
        input [3:0] target;
        begin
            encode_jump = {2'b10, 10'b0000000000, target};
        end
    endfunction

    // Class 11, bit 13 = 0: load dest <= data_memory[addr]
    function [15:0] encode_load;
        input [1:0] dest;
        input [3:0] addr;
        begin
            encode_load = {2'b11, 1'b0, 2'b00, dest, 2'b00, 3'b000, addr};
        end
    endfunction

    // Class 11, bit 13 = 1: store data_memory[addr] <= src
    function [15:0] encode_store;
        input [1:0] src;
        input [3:0] addr;
        begin
            encode_store = {2'b11, 1'b1, 2'b00, 2'b00, src, 3'b000, addr};
        end
    endfunction

    task expect_equal;
        input [127:0] label;
        input [15:0] actual;
        input [15:0] expected;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: expected %h, got %h", label, expected, actual);
                failures = failures + 1;
            end
        end
    endtask

    task write_register;
        input [1:0] addr;
        input [3:0] data;
        begin
            @(negedge clk);
            reg_write_enable = 1'b1;
            reg_write_addr = addr;
            reg_write_data = data;
            @(negedge clk);
            reg_write_enable = 1'b0;
        end
    endtask

    task write_instruction;
        input [3:0] addr;
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

    // Runs one ALU instruction, checking the combinational ALU outputs before
    // the clock edge and the pc/register writeback after it.
    task run_step;
        input [3:0] expected_pc_before;
        input [3:0] expected_result;
        input expected_zero;
        input expected_negative;
        input expected_carry;
        input expected_overflow;
        input [1:0] expected_dest;
        input [3:0] expected_writeback;
        begin
            @(negedge clk);
            run_enable = 1'b1;
            #1;
            expect_equal("pc before step", pc, expected_pc_before);
            expect_equal("result before step", result, expected_result);
            expect_equal("zero before step", zero, expected_zero);
            expect_equal("negative before step", negative, expected_negative);
            expect_equal("carry before step", carry, expected_carry);
            expect_equal("overflow before step", overflow, expected_overflow);
            @(posedge clk);
            #1;
            expect_equal("pc after step", pc, expected_pc_before + 1'b1);
            expect_equal("writeback", uut.rf.registers[expected_dest], expected_writeback);
            @(negedge clk);
            run_enable = 1'b0;
        end
    endtask

    // Runs one instruction without checking ALU-specific outputs, for
    // branch/jump/load/store steps whose effects are checked by the caller.
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
        $dumpfile("waveform.vcd");
        $dumpvars(0, top_tb);

        clk = 1'b0;
        failures = 0;
        reset = 1'b1;
        run_enable = 1'b0;
        reg_write_enable = 1'b0;
        reg_write_addr = 2'b00;
        reg_write_data = 4'b0000;
        instr_write_enable = 1'b0;
        instr_write_addr = 4'b0000;
        instr_write_data = 16'b0000_0000_0000_0000;
        dmem_write_enable = 1'b0;
        dmem_write_addr = 4'b0000;
        dmem_write_data = 4'b0000;

        #12 reset = 1'b0;

        write_register(2'd0, 4'd4);
        write_register(2'd1, 4'd5);

        write_instruction(4'd0, encode_alu(3'b000, 2'd2, 2'd0, 2'd1, 1'b0, 4'd0)); // R2 = R0 + R1
        write_instruction(4'd1, encode_alu(3'b001, 2'd3, 2'd0, 2'd1, 1'b0, 4'd0)); // R3 = R0 - R1
        write_instruction(4'd2, encode_alu(3'b010, 2'd2, 2'd2, 2'd0, 1'b1, 4'd8)); // R2 = R2 & 8
        write_instruction(4'd3, encode_alu(3'b101, 2'd2, 2'd2, 2'd0, 1'b1, 4'd1)); // R2 = R2 << 1
        write_instruction(4'd4, encode_alu(3'b111, 2'd3, 2'd0, 2'd1, 1'b0, 4'd0)); // R3 = R0 < R1
        write_instruction(4'd5, encode_store(2'd3, 4'd5));                        // mem[5] = R3
        write_instruction(4'd6, encode_load(2'd1, 4'd5));                         // R1 = mem[5]
        write_instruction(4'd7, encode_alu(3'b001, 2'd2, 2'd1, 2'd1, 1'b0, 4'd0)); // R2 = R1 - R1 (sets zero flag)
        write_instruction(4'd8, encode_branch(3'b001, 4'd3));                     // BNE +3 (not taken)
        write_instruction(4'd9, encode_branch(3'b000, 4'd3));                     // BEQ +3 (taken, skips 10-11)
        write_instruction(4'd12, encode_jump(4'd0));                              // JMP 0

        run_step(4'd0, 4'd9, 1'b0, 1'b1, 1'b0, 1'b1, 2'd2, 4'd9);
        run_step(4'd1, 4'hf, 1'b0, 1'b1, 1'b1, 1'b0, 2'd3, 4'hf);
        run_step(4'd2, 4'd8, 1'b0, 1'b1, 1'b0, 1'b0, 2'd2, 4'd8);
        run_step(4'd3, 4'd0, 1'b1, 1'b0, 1'b1, 1'b0, 2'd2, 4'd0);
        run_step(4'd4, 4'd1, 1'b0, 1'b0, 1'b0, 1'b0, 2'd3, 4'd1);

        // mem[5] = R3 (1)
        step;
        expect_equal("pc after store", pc, 4'd6);
        expect_equal("dmem[5] after store", uut.dmem.memory[5], 4'd1);

        // R1 = mem[5] (1)
        step;
        expect_equal("pc after load", pc, 4'd7);
        expect_equal("R1 after load", uut.rf.registers[1], 4'd1);

        // R2 = R1 - R1 = 0, sets zero flag for the branches below
        step;
        expect_equal("pc after sub", pc, 4'd8);
        expect_equal("R2 after sub", uut.rf.registers[2], 4'd0);

        // BNE +3: zero flag is set, so the branch is not taken
        step;
        expect_equal("pc after bne not taken", pc, 4'd9);

        // BEQ +3: zero flag is set, so the branch is taken (skips 10-11)
        step;
        expect_equal("pc after beq taken", pc, 4'd12);
        expect_equal("R0 unchanged by skipped instructions", uut.rf.registers[0], 4'd4);

        // JMP 0
        step;
        expect_equal("pc after jump", pc, 4'd0);

        if (failures == 0) begin
            $display("top_tb: PASS");
        end else begin
            $display("top_tb: FAIL (%0d failures)", failures);
            $finish_and_return(1);
        end

        $finish;
    end
endmodule
