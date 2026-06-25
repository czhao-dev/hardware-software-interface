// Decodes raw 32-bit RV32I instruction words into a structured form.
#pragma once

#include <cstdint>
#include <string>

namespace riscv_sim {

// Opcode (bits [6:0]) values for each instruction format/group.
constexpr std::uint32_t OP_LUI = 0b0110111;
constexpr std::uint32_t OP_AUIPC = 0b0010111;
constexpr std::uint32_t OP_JAL = 0b1101111;
constexpr std::uint32_t OP_JALR = 0b1100111;
constexpr std::uint32_t OP_BRANCH = 0b1100011;
constexpr std::uint32_t OP_LOAD = 0b0000011;
constexpr std::uint32_t OP_STORE = 0b0100011;
constexpr std::uint32_t OP_IMM = 0b0010011;
constexpr std::uint32_t OP_REG = 0b0110011;
constexpr std::uint32_t OP_FENCE = 0b0001111;
constexpr std::uint32_t OP_SYSTEM = 0b1110011;

// A decoded RV32I instruction.
struct Instruction {
    std::uint32_t raw = 0;
    std::string fmt;
    std::string mnemonic;
    std::uint32_t opcode = 0;
    int rd = 0;
    int rs1 = 0;
    int rs2 = 0;
    int funct3 = 0;
    int funct7 = 0;
    std::int32_t imm = 0;

    std::string to_string() const;
};

// Decode a 32-bit instruction word. Throws IllegalInstructionError if it
// cannot be decoded.
Instruction decode(std::uint32_t raw);

}  // namespace riscv_sim
