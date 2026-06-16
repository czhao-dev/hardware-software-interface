`timescale 1ns / 1ps

module instruction_memory #(parameter PROGRAM_FILE = "") (
    input wire clk,
    input wire reset,
    input wire write_enable,
    input wire [7:0] write_addr,
    input wire [15:0] write_data,
    input wire [7:0] read_addr,
    output wire [15:0] instruction
);
    reg [15:0] memory [0:255];
    integer i;

    // Initialize memory once at simulation start: load from hex file
    // when PROGRAM_FILE is set, otherwise zero-initialize.  The reset
    // signal does NOT clear memory so that a loaded program survives
    // the testbench reset cycle.
    initial begin
        for (i = 0; i < 256; i = i + 1)
            memory[i] = 16'h0000;
        if (PROGRAM_FILE != "")
            $readmemh(PROGRAM_FILE, memory);
    end

    always @(posedge clk) begin
        if (write_enable)
            memory[write_addr] <= write_data;
    end

    assign instruction = memory[read_addr];
endmodule
