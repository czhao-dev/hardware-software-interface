`timescale 1ns / 1ps

// Pipeline register separating the IF and EX-WB stages.
// On reset or flush, the output is forced to NOP so the EX-WB stage
// performs no side effects.  The NOP encoding is SYSTEM sysop=00
// (class 11, subop 11, sysop 00 = 0xF000).
module if_exwb_reg (
    input wire clk,
    input wire reset,
    input wire enable,      // cpu_step: freezes the register when low
    input wire flush,       // redirect: inject NOP instead of real instruction
    input wire [15:0] instr_in,
    input wire [7:0]  pc_in,
    output reg [15:0] instr_out,
    output reg [7:0]  pc_out
);
    localparam [15:0] NOP = 16'hF000;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            instr_out <= NOP;
            pc_out    <= 8'h00;
        end else if (enable) begin
            instr_out <= flush ? NOP : instr_in;
            pc_out    <= pc_in;
        end
    end
endmodule
