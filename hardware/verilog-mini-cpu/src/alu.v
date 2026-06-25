`timescale 1ns / 1ps

module alu (
    input wire [7:0] a,
    input wire [7:0] b,
    input wire [3:0] op,
    output reg [7:0] result,
    output wire zero,
    output wire negative,
    output reg carry,
    output reg overflow
);
    reg [8:0] extended_result;
    reg [15:0] wide;
    reg [15:0] product;

    always @(*) begin
        result = 8'h00;
        carry = 1'b0;
        overflow = 1'b0;
        extended_result = 9'h000;
        wide = 16'h0000;
        product = 16'h0000;

        case (op)
            4'b0000: begin // ADD
                extended_result = {1'b0, a} + {1'b0, b};
                result = extended_result[7:0];
                carry = extended_result[8];
                overflow = (a[7] == b[7]) && (result[7] != a[7]);
            end
            4'b0001: begin // SUB
                extended_result = {1'b0, a} - {1'b0, b};
                result = extended_result[7:0];
                carry = (a < b);
                overflow = (a[7] != b[7]) && (result[7] != a[7]);
            end
            4'b0010: result = a & b; // AND
            4'b0011: result = a | b; // OR
            4'b0100: result = a ^ b; // XOR
            4'b0101: begin // SLL: a << b[2:0]
                wide = {8'h00, a} << b[2:0];
                result = wide[7:0];
                carry = wide[8];
            end
            4'b0110: begin // SRL: a >> b[2:0]
                wide = {a, 8'h00} >> b[2:0];
                result = wide[15:8];
                carry = wide[7];
            end
            4'b0111: result = (a < b) ? 8'h01 : 8'h00; // SLT (unsigned)
            4'b1000: result = ~a; // NOT (unary, b ignored)
            4'b1001: begin // NEG: 0 - a (unary, b ignored)
                result = (~a) + 8'h01;
                carry = (a != 8'h00);
                overflow = (a == 8'h80);
            end
            4'b1010: begin // ROL: rotate-left a by b[2:0]
                wide = {a, a} << b[2:0];
                result = wide[15:8];
                carry = (b[2:0] == 3'b000) ? 1'b0 : a[3'd0 - b[2:0]];
            end
            4'b1011: begin // ROR: rotate-right a by b[2:0]
                wide = {a, a} >> b[2:0];
                result = wide[7:0];
                carry = (b[2:0] == 3'b000) ? 1'b0 : a[b[2:0] - 3'd1];
            end
            4'b1100: begin // MUL: low byte of a * b
                product = a * b;
                result = product[7:0];
                carry = |product[15:8];
            end
            4'b1101: result = ~(a & b); // NAND
            4'b1110: begin // INC: a + 1 (unary, b ignored)
                extended_result = {1'b0, a} + 9'h001;
                result = extended_result[7:0];
                carry = extended_result[8];
                overflow = (a == 8'h7f);
            end
            4'b1111: begin // DEC: a - 1 (unary, b ignored)
                result = a - 8'h01;
                carry = (a == 8'h00);
                overflow = (a == 8'h80);
            end
        endcase
    end

    assign zero = (result == 8'h00);
    assign negative = result[7];

endmodule
