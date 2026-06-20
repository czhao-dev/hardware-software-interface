#include "riscv_sim/decoder.hpp"

#include <cstdio>
#include <map>

#include "riscv_sim/bits.hpp"
#include "riscv_sim/errors.hpp"

namespace riscv_sim {

namespace {

const std::map<int, std::string> kBranchMnemonics = {
    {0b000, "beq"}, {0b001, "bne"}, {0b100, "blt"},
    {0b101, "bge"}, {0b110, "bltu"}, {0b111, "bgeu"},
};
const std::map<int, std::string> kLoadMnemonics = {
    {0b000, "lb"}, {0b001, "lh"}, {0b010, "lw"}, {0b100, "lbu"}, {0b101, "lhu"},
};
const std::map<int, std::string> kStoreMnemonics = {
    {0b000, "sb"}, {0b001, "sh"}, {0b010, "sw"},
};
const std::map<int, std::string> kImmMnemonics = {
    {0b000, "addi"}, {0b010, "slti"}, {0b011, "sltiu"}, {0b100, "xori"},
    {0b110, "ori"}, {0b111, "andi"}, {0b001, "slli"}, {0b101, "srli"},
};
const std::map<std::pair<int, int>, std::string> kRegMnemonics = {
    {{0b000, 0b0000000}, "add"}, {{0b000, 0b0100000}, "sub"},
    {{0b001, 0b0000000}, "sll"}, {{0b010, 0b0000000}, "slt"},
    {{0b011, 0b0000000}, "sltu"}, {{0b100, 0b0000000}, "xor"},
    {{0b101, 0b0000000}, "srl"}, {{0b101, 0b0100000}, "sra"},
    {{0b110, 0b0000000}, "or"}, {{0b111, 0b0000000}, "and"},
};

std::string hex32(std::uint32_t value) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "0x%08x", value);
    return buf;
}

std::string binary(std::uint32_t value, int width) {
    std::string s(width, '0');
    for (int i = width - 1; i >= 0; --i) {
        if (value & (1u << i)) s[width - 1 - i] = '1';
    }
    return "0b" + s;
}

bool is_one_of(const std::string& m, std::initializer_list<const char*> options) {
    for (const char* opt : options) {
        if (m == opt) return true;
    }
    return false;
}

}  // namespace

std::string Instruction::to_string() const {
    std::string rd_s = "x" + std::to_string(rd);
    std::string rs1_s = "x" + std::to_string(rs1);
    std::string rs2_s = "x" + std::to_string(rs2);
    std::string imm_s = std::to_string(imm);

    if (fmt == "R") {
        return mnemonic + " " + rd_s + ", " + rs1_s + ", " + rs2_s;
    }
    if (fmt == "I") {
        if (is_one_of(mnemonic, {"jalr", "lb", "lh", "lw", "lbu", "lhu"})) {
            return mnemonic + " " + rd_s + ", " + imm_s + "(" + rs1_s + ")";
        }
        return mnemonic + " " + rd_s + ", " + rs1_s + ", " + imm_s;
    }
    if (fmt == "S") {
        return mnemonic + " " + rs2_s + ", " + imm_s + "(" + rs1_s + ")";
    }
    if (fmt == "B") {
        return mnemonic + " " + rs1_s + ", " + rs2_s + ", " + imm_s;
    }
    if (fmt == "U") {
        return mnemonic + " " + rd_s + ", " + imm_s;
    }
    if (fmt == "J") {
        return mnemonic + " " + rd_s + ", " + imm_s;
    }
    if (fmt == "SYSTEM") {
        return mnemonic;
    }
    return mnemonic;
}

Instruction decode(std::uint32_t raw) {
    std::uint32_t opcode = raw & 0x7F;
    int rd = (raw >> 7) & 0x1F;
    int funct3 = (raw >> 12) & 0x7;
    int rs1 = (raw >> 15) & 0x1F;
    int rs2 = (raw >> 20) & 0x1F;
    int funct7 = (raw >> 25) & 0x7F;

    Instruction instr;
    instr.raw = raw;
    instr.opcode = opcode;

    if (opcode == OP_LUI) {
        instr.fmt = "U";
        instr.mnemonic = "lui";
        instr.rd = rd;
        instr.imm = sign_extend(raw & 0xFFFFF000u, 32);
        return instr;
    }

    if (opcode == OP_AUIPC) {
        instr.fmt = "U";
        instr.mnemonic = "auipc";
        instr.rd = rd;
        instr.imm = sign_extend(raw & 0xFFFFF000u, 32);
        return instr;
    }

    if (opcode == OP_JAL) {
        instr.fmt = "J";
        instr.mnemonic = "jal";
        instr.rd = rd;
        std::uint32_t imm_bits = ((raw >> 21) & 0x3FFu) << 1 | ((raw >> 20) & 0x1u) << 11 |
                                  ((raw >> 12) & 0xFFu) << 12 | ((raw >> 31) & 0x1u) << 20;
        instr.imm = sign_extend(imm_bits, 21);
        return instr;
    }

    if (opcode == OP_JALR) {
        instr.fmt = "I";
        instr.mnemonic = "jalr";
        instr.rd = rd;
        instr.rs1 = rs1;
        instr.funct3 = funct3;
        instr.imm = sign_extend(raw >> 20, 12);
        return instr;
    }

    if (opcode == OP_BRANCH) {
        auto it = kBranchMnemonics.find(funct3);
        if (it == kBranchMnemonics.end()) {
            throw IllegalInstructionError("unknown branch funct3=" + binary(funct3, 3) + " in " + hex32(raw));
        }
        instr.fmt = "B";
        instr.mnemonic = it->second;
        instr.rs1 = rs1;
        instr.rs2 = rs2;
        instr.funct3 = funct3;
        std::uint32_t imm_bits = ((raw >> 8) & 0xFu) << 1 | ((raw >> 25) & 0x3Fu) << 5 |
                                  ((raw >> 7) & 0x1u) << 11 | ((raw >> 31) & 0x1u) << 12;
        instr.imm = sign_extend(imm_bits, 13);
        return instr;
    }

    if (opcode == OP_LOAD) {
        auto it = kLoadMnemonics.find(funct3);
        if (it == kLoadMnemonics.end()) {
            throw IllegalInstructionError("unknown load funct3=" + binary(funct3, 3) + " in " + hex32(raw));
        }
        instr.fmt = "I";
        instr.mnemonic = it->second;
        instr.rd = rd;
        instr.rs1 = rs1;
        instr.funct3 = funct3;
        instr.imm = sign_extend(raw >> 20, 12);
        return instr;
    }

    if (opcode == OP_STORE) {
        auto it = kStoreMnemonics.find(funct3);
        if (it == kStoreMnemonics.end()) {
            throw IllegalInstructionError("unknown store funct3=" + binary(funct3, 3) + " in " + hex32(raw));
        }
        instr.fmt = "S";
        instr.mnemonic = it->second;
        instr.rs1 = rs1;
        instr.rs2 = rs2;
        instr.funct3 = funct3;
        std::uint32_t imm_bits = ((raw >> 7) & 0x1Fu) | (((raw >> 25) & 0x7Fu) << 5);
        instr.imm = sign_extend(imm_bits, 12);
        return instr;
    }

    if (opcode == OP_IMM) {
        auto it = kImmMnemonics.find(funct3);
        if (it == kImmMnemonics.end()) {
            throw IllegalInstructionError("unknown OP-IMM funct3=" + binary(funct3, 3) + " in " + hex32(raw));
        }
        std::string mnemonic = it->second;
        if (funct3 == 0b101) {
            mnemonic = (funct7 == 0b0100000) ? "srai" : "srli";
        }
        std::int32_t imm;
        if (mnemonic == "slli" || mnemonic == "srli" || mnemonic == "srai") {
            imm = rs2;  // shift amount lives in the rs2 field (imm[4:0])
        } else {
            imm = sign_extend(raw >> 20, 12);
        }
        instr.fmt = "I";
        instr.mnemonic = mnemonic;
        instr.rd = rd;
        instr.rs1 = rs1;
        instr.funct3 = funct3;
        instr.funct7 = funct7;
        instr.imm = imm;
        return instr;
    }

    if (opcode == OP_REG) {
        auto it = kRegMnemonics.find({funct3, funct7});
        if (it == kRegMnemonics.end()) {
            throw IllegalInstructionError("unknown OP funct3=" + binary(funct3, 3) + " funct7=" +
                                           binary(funct7, 7) + " in " + hex32(raw));
        }
        instr.fmt = "R";
        instr.mnemonic = it->second;
        instr.rd = rd;
        instr.rs1 = rs1;
        instr.rs2 = rs2;
        instr.funct3 = funct3;
        instr.funct7 = funct7;
        return instr;
    }

    if (opcode == OP_FENCE) {
        instr.fmt = "FENCE";
        instr.mnemonic = "fence";
        return instr;
    }

    if (opcode == OP_SYSTEM) {
        std::uint32_t imm = raw >> 20;
        instr.fmt = "SYSTEM";
        instr.mnemonic = (imm == 1) ? "ebreak" : "ecall";
        instr.imm = static_cast<std::int32_t>(imm);
        return instr;
    }

    throw IllegalInstructionError("unknown opcode " + binary(opcode, 7) + " in " + hex32(raw));
}

}  // namespace riscv_sim
