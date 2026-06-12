`timescale 1ns / 1ps

module control_unit (
    input wire [15:0] instruction,
    output wire [1:0] instr_class,
    output wire [2:0] alu_op,
    output wire [1:0] dest_addr,
    output wire [1:0] src_a_addr,
    output wire [1:0] src_b_addr,
    output wire use_immediate,
    output wire [3:0] immediate
);
    // [15:14] instruction class: 00=ALU, 01=branch, 10=jump, 11=load/store
    // [13:11] ALU op, or branch condition, or load/store flag in bit 13
    // [10:9]  destination register (ALU dest, load dest)
    // [8:7]   source register A (ALU src_a, store src)
    // [6:5]   source register B (ALU src_b)
    // [4]     use_immediate (ALU only)
    // [3:0]   immediate, or branch offset, or jump target, or memory address
    assign instr_class   = instruction[15:14];
    assign alu_op        = instruction[13:11];
    assign dest_addr      = instruction[10:9];
    assign src_a_addr     = instruction[8:7];
    assign src_b_addr     = instruction[6:5];
    assign use_immediate = instruction[4];
    assign immediate     = instruction[3:0];
endmodule
