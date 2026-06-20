#include "riscv_sim/encoder.hpp"

#include "riscv_sim/decoder.hpp"

namespace riscv_sim::encoder {

namespace {
std::uint32_t u(std::int32_t value, int bits) {
    std::uint32_t mask = (bits == 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    return static_cast<std::uint32_t>(value) & mask;
}
}  // namespace

std::uint32_t r_type(std::uint32_t opcode, int rd, int funct3, int rs1, int rs2, int funct7) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

std::uint32_t i_type(std::uint32_t opcode, int rd, int funct3, int rs1, std::int32_t imm) {
    return (u(imm, 12) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

std::uint32_t s_type(std::uint32_t opcode, int funct3, int rs1, int rs2, std::int32_t imm) {
    std::uint32_t uimm = u(imm, 12);
    return ((uimm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | ((uimm & 0x1F) << 7) | opcode;
}

std::uint32_t b_type(std::uint32_t opcode, int funct3, int rs1, int rs2, std::int32_t imm) {
    std::uint32_t uimm = u(imm, 13);
    std::uint32_t bit12 = (uimm >> 12) & 0x1;
    std::uint32_t bit11 = (uimm >> 11) & 0x1;
    std::uint32_t bits10_5 = (uimm >> 5) & 0x3F;
    std::uint32_t bits4_1 = (uimm >> 1) & 0xF;
    return (bit12 << 31) | (bits10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (bits4_1 << 8) |
           (bit11 << 7) | opcode;
}

std::uint32_t u_type(std::uint32_t opcode, int rd, std::int32_t imm) {
    return (u(imm, 32) & 0xFFFFF000u) | (rd << 7) | opcode;
}

std::uint32_t j_type(std::uint32_t opcode, int rd, std::int32_t imm) {
    std::uint32_t uimm = u(imm, 21);
    std::uint32_t bit20 = (uimm >> 20) & 0x1;
    std::uint32_t bits10_1 = (uimm >> 1) & 0x3FF;
    std::uint32_t bit11 = (uimm >> 11) & 0x1;
    std::uint32_t bits19_12 = (uimm >> 12) & 0xFF;
    return (bit20 << 31) | (bits10_1 << 21) | (bit11 << 20) | (bits19_12 << 12) | (rd << 7) | opcode;
}

std::uint32_t add(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b000, rs1, rs2, 0b0000000); }
std::uint32_t sub(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b000, rs1, rs2, 0b0100000); }
std::uint32_t sll(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b001, rs1, rs2, 0b0000000); }
std::uint32_t slt(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b010, rs1, rs2, 0b0000000); }
std::uint32_t sltu(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b011, rs1, rs2, 0b0000000); }
std::uint32_t xor_(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b100, rs1, rs2, 0b0000000); }
std::uint32_t srl(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b101, rs1, rs2, 0b0000000); }
std::uint32_t sra(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b101, rs1, rs2, 0b0100000); }
std::uint32_t or_(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b110, rs1, rs2, 0b0000000); }
std::uint32_t and_(int rd, int rs1, int rs2) { return r_type(OP_REG, rd, 0b111, rs1, rs2, 0b0000000); }

std::uint32_t addi(int rd, int rs1, std::int32_t imm) { return i_type(OP_IMM, rd, 0b000, rs1, imm); }
std::uint32_t slti(int rd, int rs1, std::int32_t imm) { return i_type(OP_IMM, rd, 0b010, rs1, imm); }
std::uint32_t sltiu(int rd, int rs1, std::int32_t imm) { return i_type(OP_IMM, rd, 0b011, rs1, imm); }
std::uint32_t xori(int rd, int rs1, std::int32_t imm) { return i_type(OP_IMM, rd, 0b100, rs1, imm); }
std::uint32_t ori(int rd, int rs1, std::int32_t imm) { return i_type(OP_IMM, rd, 0b110, rs1, imm); }
std::uint32_t andi(int rd, int rs1, std::int32_t imm) { return i_type(OP_IMM, rd, 0b111, rs1, imm); }
std::uint32_t slli(int rd, int rs1, int shamt) { return r_type(OP_IMM, rd, 0b001, rs1, shamt, 0b0000000); }
std::uint32_t srli(int rd, int rs1, int shamt) { return r_type(OP_IMM, rd, 0b101, rs1, shamt, 0b0000000); }
std::uint32_t srai(int rd, int rs1, int shamt) { return r_type(OP_IMM, rd, 0b101, rs1, shamt, 0b0100000); }

std::uint32_t lb(int rd, int rs1, std::int32_t imm) { return i_type(OP_LOAD, rd, 0b000, rs1, imm); }
std::uint32_t lh(int rd, int rs1, std::int32_t imm) { return i_type(OP_LOAD, rd, 0b001, rs1, imm); }
std::uint32_t lw(int rd, int rs1, std::int32_t imm) { return i_type(OP_LOAD, rd, 0b010, rs1, imm); }
std::uint32_t lbu(int rd, int rs1, std::int32_t imm) { return i_type(OP_LOAD, rd, 0b100, rs1, imm); }
std::uint32_t lhu(int rd, int rs1, std::int32_t imm) { return i_type(OP_LOAD, rd, 0b101, rs1, imm); }

std::uint32_t sb(int rs1, int rs2, std::int32_t imm) { return s_type(OP_STORE, 0b000, rs1, rs2, imm); }
std::uint32_t sh(int rs1, int rs2, std::int32_t imm) { return s_type(OP_STORE, 0b001, rs1, rs2, imm); }
std::uint32_t sw(int rs1, int rs2, std::int32_t imm) { return s_type(OP_STORE, 0b010, rs1, rs2, imm); }

std::uint32_t beq(int rs1, int rs2, std::int32_t imm) { return b_type(OP_BRANCH, 0b000, rs1, rs2, imm); }
std::uint32_t bne(int rs1, int rs2, std::int32_t imm) { return b_type(OP_BRANCH, 0b001, rs1, rs2, imm); }
std::uint32_t blt(int rs1, int rs2, std::int32_t imm) { return b_type(OP_BRANCH, 0b100, rs1, rs2, imm); }
std::uint32_t bge(int rs1, int rs2, std::int32_t imm) { return b_type(OP_BRANCH, 0b101, rs1, rs2, imm); }
std::uint32_t bltu(int rs1, int rs2, std::int32_t imm) { return b_type(OP_BRANCH, 0b110, rs1, rs2, imm); }
std::uint32_t bgeu(int rs1, int rs2, std::int32_t imm) { return b_type(OP_BRANCH, 0b111, rs1, rs2, imm); }

std::uint32_t lui(int rd, std::int32_t imm) { return u_type(OP_LUI, rd, imm); }
std::uint32_t auipc(int rd, std::int32_t imm) { return u_type(OP_AUIPC, rd, imm); }

std::uint32_t jal(int rd, std::int32_t imm) { return j_type(OP_JAL, rd, imm); }
std::uint32_t jalr(int rd, int rs1, std::int32_t imm) { return i_type(OP_JALR, rd, 0b000, rs1, imm); }

std::uint32_t ecall() { return i_type(OP_SYSTEM, 0, 0b000, 0, 0); }
std::uint32_t ebreak() { return i_type(OP_SYSTEM, 0, 0b000, 0, 1); }

}  // namespace riscv_sim::encoder
