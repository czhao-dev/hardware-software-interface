`timescale 1ns / 1ps

// End-to-end testbench: loads examples/loop_sum.hex via $readmemh and
// free-runs until the HALT instruction sets halted=1, then checks the
// expected final register values.
//
// Expected outcome after loop_sum (sum 1+2+...+10):
//   R0 = 55  (0x37)   -- accumulator
//   R1 =  0  (0x00)   -- loop counter exhausted
//
// Execution takes ~43 pipeline cycles; the timeout is set to 200.

module top_program_tb;
    reg clk;
    reg reset;

    wire [7:0] pc;
    wire [15:0] instruction;
    wire [7:0] result;
    wire zero, negative, carry, overflow;
    wire halted;

    integer failures;
    integer cycle_count;

    top #(.PROGRAM_FILE("examples/loop_sum.hex")) uut (
        .clk(clk),
        .reset(reset),
        .run_enable(1'b1),
        .reg_write_enable(1'b0),
        .reg_write_addr(3'b000),
        .reg_write_data(8'h00),
        .instr_write_enable(1'b0),
        .instr_write_addr(8'h00),
        .instr_write_data(16'h0000),
        .dmem_write_enable(1'b0),
        .dmem_write_addr(8'h00),
        .dmem_write_data(8'h00),
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

    task expect_equal;
        input [63:0] label;
        input [7:0] actual;
        input [7:0] expected;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: expected 0x%02h, got 0x%02h", label, expected, actual);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        clk = 1'b0;
        failures = 0;
        reset = 1'b1;

        $dumpfile("waveform_program.vcd");
        $dumpvars(0, top_program_tb);

        #12 reset = 1'b0;

        // Wait for halted with a cycle-count timeout.
        begin : wait_loop
            integer i;
            for (i = 0; i < 200; i = i + 1) begin
                if (halted) disable wait_loop;
                @(posedge clk);
                #1;
            end
            $display("top_program_tb: TIMEOUT — halted never set after 200 cycles");
            $finish_and_return(1);
        end

        expect_equal("R0 (sum)", uut.rf.registers[0], 8'h37);
        expect_equal("R1 (counter)", uut.rf.registers[1], 8'h00);

        if (failures == 0) begin
            $display("top_program_tb: PASS");
        end else begin
            $display("top_program_tb: FAIL (%0d failures)", failures);
            $finish_and_return(1);
        end

        $finish;
    end
endmodule
