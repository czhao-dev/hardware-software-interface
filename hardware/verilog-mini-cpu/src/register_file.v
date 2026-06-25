`timescale 1ns / 1ps

module register_file (
    input wire clk,
    input wire reset,
    input wire write_enable,
    input wire [2:0] write_addr,
    input wire [7:0] write_data,
    input wire [2:0] read_addr_a,
    input wire [2:0] read_addr_b,
    output wire [7:0] reg_a,
    output wire [7:0] reg_b
);
    reg [7:0] registers [0:7];
    integer i;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            for (i = 0; i < 8; i = i + 1) begin
                registers[i] <= 8'h00;
            end
        end else if (write_enable) begin
            registers[write_addr] <= write_data;
        end
    end

    assign reg_a = registers[read_addr_a];
    assign reg_b = registers[read_addr_b];
endmodule
