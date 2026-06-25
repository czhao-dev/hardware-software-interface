`timescale 1ns / 1ps

module control_unit (
    input wire [15:0] instruction,
    output wire [1:0] instr_class,
    output wire [3:0] alu_op,
    output wire [2:0] alu_reg_dest,
    output wire [2:0] alu_reg_src_a,
    output wire [2:0] alu_reg_src_b,
    output wire [2:0] alu_imm_dest,
    output wire [7:0] immediate,
    output wire [2:0] branch_cond,
    output wire [7:0] branch_offset,
    output wire [7:0] jump_target,
    output wire jump_link,
    output wire [2:0] mem_reg,
    output wire [7:0] mem_addr,
    output wire is_alu_reg,
    output wire is_alu_imm,
    output wire is_ldi,
    output wire is_branch,
    output wire is_jump,
    output wire is_load,
    output wire is_store,
    output wire is_halt,
    output wire is_ret
);
    // [15:14] instruction class: 00=ALU-REG, 01=ALU-IMM, 10=BRANCH, 11=JUMP/MEM/SYSTEM
    wire [1:0] subop11 = instruction[13:12];
    wire [2:0] alu_imm_subop = instruction[13:11];
    wire [1:0] sysop = instruction[11:10];

    assign instr_class = instruction[15:14];

    // Class 00 (ALU-REG): dest <= src_a OP(alu_op) src_b
    assign alu_reg_dest  = instruction[9:7];
    assign alu_reg_src_a = instruction[6:4];
    assign alu_reg_src_b = instruction[3:1];

    // Class 01 (ALU-IMM, accumulator style): dest <= dest OP imm, or dest <= imm for LDI
    assign alu_imm_dest = instruction[10:8];
    assign immediate    = instruction[7:0];

    // Class-01 subops 001-110 (ADDI/SUBI/ANDI/ORI/XORI/SLTI) reuse the
    // class-00 ALU opcodes/flag logic; LDI (000) bypasses the ALU.
    wire [3:0] alu_imm_op = (alu_imm_subop == 3'b001) ? 4'b0000 :  // ADDI -> ADD
                            (alu_imm_subop == 3'b010) ? 4'b0001 :  // SUBI -> SUB
                            (alu_imm_subop == 3'b011) ? 4'b0010 :  // ANDI -> AND
                            (alu_imm_subop == 3'b100) ? 4'b0011 :  // ORI  -> OR
                            (alu_imm_subop == 3'b101) ? 4'b0100 :  // XORI -> XOR
                            (alu_imm_subop == 3'b110) ? 4'b0111 :  // SLTI -> SLT
                            4'b0000;                               // LDI / reserved

    assign alu_op = (instr_class == 2'b00) ? instruction[13:10] : alu_imm_op;

    // Class 10 (BRANCH): pc <= pc + offset (signed) when cond is met, else pc + 1
    assign branch_cond   = instruction[13:11];
    assign branch_offset = instruction[10:3];

    // Class 11 (JUMP/MEM/SYSTEM)
    assign jump_target = instruction[11:4];
    assign jump_link   = instruction[3];
    assign mem_reg     = instruction[11:9];
    assign mem_addr    = instruction[8:1];

    assign is_alu_reg = (instr_class == 2'b00);
    assign is_alu_imm = (instr_class == 2'b01);
    assign is_ldi     = is_alu_imm & (alu_imm_subop == 3'b000);
    assign is_branch  = (instr_class == 2'b10);
    assign is_jump    = (instr_class == 2'b11) & (subop11 == 2'b00);
    assign is_load    = (instr_class == 2'b11) & (subop11 == 2'b01);
    assign is_store   = (instr_class == 2'b11) & (subop11 == 2'b10);
    assign is_halt    = (instr_class == 2'b11) & (subop11 == 2'b11) & (sysop == 2'b01);
    assign is_ret     = (instr_class == 2'b11) & (subop11 == 2'b11) & (sysop == 2'b10);
endmodule
