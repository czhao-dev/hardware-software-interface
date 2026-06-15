`timescale 1ns / 1ps

module data_memory (
    input wire clk,
    input wire reset,
    input wire write_enable,
    input wire [7:0] write_addr,
    input wire [7:0] write_data,
    input wire [7:0] read_addr,
    output wire [7:0] read_data
);
    reg [7:0] memory [0:255];
    integer i;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            for (i = 0; i < 256; i = i + 1) begin
                memory[i] <= 8'h00;
            end
        end else if (write_enable) begin
            memory[write_addr] <= write_data;
        end
    end

    assign read_data = memory[read_addr];
endmodule
