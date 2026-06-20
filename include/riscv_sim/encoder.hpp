// Encodes RV32I mnemonics into raw 32-bit instruction words.
//
// This is a minimal assembler used to build test fixtures and example
// programs; it is not a general-purpose RISC-V assembler.
#pragma once

#include <cstdint>

namespace riscv_sim::encoder {

std::uint32_t r_type(std::uint32_t opcode, int rd, int funct3, int rs1, int rs2, int funct7);
std::uint32_t i_type(std::uint32_t opcode, int rd, int funct3, int rs1, std::int32_t imm);
std::uint32_t s_type(std::uint32_t opcode, int funct3, int rs1, int rs2, std::int32_t imm);
std::uint32_t b_type(std::uint32_t opcode, int funct3, int rs1, int rs2, std::int32_t imm);
std::uint32_t u_type(std::uint32_t opcode, int rd, std::int32_t imm);
std::uint32_t j_type(std::uint32_t opcode, int rd, std::int32_t imm);

std::uint32_t add(int rd, int rs1, int rs2);
std::uint32_t sub(int rd, int rs1, int rs2);
std::uint32_t sll(int rd, int rs1, int rs2);
std::uint32_t slt(int rd, int rs1, int rs2);
std::uint32_t sltu(int rd, int rs1, int rs2);
std::uint32_t xor_(int rd, int rs1, int rs2);
std::uint32_t srl(int rd, int rs1, int rs2);
std::uint32_t sra(int rd, int rs1, int rs2);
std::uint32_t or_(int rd, int rs1, int rs2);
std::uint32_t and_(int rd, int rs1, int rs2);

std::uint32_t addi(int rd, int rs1, std::int32_t imm);
std::uint32_t slti(int rd, int rs1, std::int32_t imm);
std::uint32_t sltiu(int rd, int rs1, std::int32_t imm);
std::uint32_t xori(int rd, int rs1, std::int32_t imm);
std::uint32_t ori(int rd, int rs1, std::int32_t imm);
std::uint32_t andi(int rd, int rs1, std::int32_t imm);
std::uint32_t slli(int rd, int rs1, int shamt);
std::uint32_t srli(int rd, int rs1, int shamt);
std::uint32_t srai(int rd, int rs1, int shamt);

std::uint32_t lb(int rd, int rs1, std::int32_t imm);
std::uint32_t lh(int rd, int rs1, std::int32_t imm);
std::uint32_t lw(int rd, int rs1, std::int32_t imm);
std::uint32_t lbu(int rd, int rs1, std::int32_t imm);
std::uint32_t lhu(int rd, int rs1, std::int32_t imm);

std::uint32_t sb(int rs1, int rs2, std::int32_t imm);
std::uint32_t sh(int rs1, int rs2, std::int32_t imm);
std::uint32_t sw(int rs1, int rs2, std::int32_t imm);

std::uint32_t beq(int rs1, int rs2, std::int32_t imm);
std::uint32_t bne(int rs1, int rs2, std::int32_t imm);
std::uint32_t blt(int rs1, int rs2, std::int32_t imm);
std::uint32_t bge(int rs1, int rs2, std::int32_t imm);
std::uint32_t bltu(int rs1, int rs2, std::int32_t imm);
std::uint32_t bgeu(int rs1, int rs2, std::int32_t imm);

std::uint32_t lui(int rd, std::int32_t imm);
std::uint32_t auipc(int rd, std::int32_t imm);

std::uint32_t jal(int rd, std::int32_t imm);
std::uint32_t jalr(int rd, int rs1, std::int32_t imm);

std::uint32_t ecall();
std::uint32_t ebreak();

}  // namespace riscv_sim::encoder
