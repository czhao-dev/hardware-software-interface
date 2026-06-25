#include "riscv_sim/decoder.hpp"

#include "riscv_sim/encoder.hpp"
#include "riscv_sim/errors.hpp"
#include "test_framework.hpp"

namespace e = riscv_sim::encoder;
using riscv_sim::decode;
using riscv_sim::IllegalInstructionError;

TEST_CASE("decode_r_type_add") {
    auto instr = decode(e::add(3, 1, 2));
    CHECK(instr.fmt == "R");
    CHECK(instr.mnemonic == "add");
    CHECK(instr.rd == 3);
    CHECK(instr.rs1 == 1);
    CHECK(instr.rs2 == 2);
}

TEST_CASE("decode_i_type_addi_positive_and_negative_immediate") {
    auto pos = decode(e::addi(1, 0, 5));
    CHECK(pos.mnemonic == "addi");
    CHECK(pos.rd == 1);
    CHECK(pos.rs1 == 0);
    CHECK(pos.imm == 5);

    auto neg = decode(e::addi(1, 1, -1));
    CHECK(neg.imm == -1);
}

TEST_CASE("decode_shift_immediate_uses_shamt_field") {
    auto instr = decode(e::slli(5, 1, 3));
    CHECK(instr.mnemonic == "slli");
    CHECK(instr.imm == 3);
}

TEST_CASE("decode_s_type_store") {
    auto instr = decode(e::sw(2, 1, 0x40));
    CHECK(instr.fmt == "S");
    CHECK(instr.mnemonic == "sw");
    CHECK(instr.rs1 == 2);
    CHECK(instr.rs2 == 1);
    CHECK(instr.imm == 0x40);
}

TEST_CASE("decode_b_type_branch_negative_offset") {
    auto instr = decode(e::bne(1, 0, -8));
    CHECK(instr.fmt == "B");
    CHECK(instr.mnemonic == "bne");
    CHECK(instr.imm == -8);
}

TEST_CASE("decode_u_type_lui") {
    auto instr = decode(e::lui(5, 0x12345000));
    CHECK(instr.fmt == "U");
    CHECK(instr.imm == 0x12345000);
}

TEST_CASE("decode_j_type_jal") {
    auto instr = decode(e::jal(1, 0x800));
    CHECK(instr.fmt == "J");
    CHECK(instr.mnemonic == "jal");
    CHECK(instr.imm == 0x800);
}

TEST_CASE("decode_system_instructions") {
    CHECK(decode(e::ecall()).mnemonic == "ecall");
    CHECK(decode(e::ebreak()).mnemonic == "ebreak");
}

TEST_CASE("decode_unknown_opcode_raises") {
    CHECK_THROWS_AS(decode(0b1111111), IllegalInstructionError);
}

TEST_CASE("instruction_str_formats_operands") {
    CHECK(decode(e::addi(1, 0, 5)).to_string() == "addi x1, x0, 5");
    CHECK(decode(e::add(3, 1, 2)).to_string() == "add x3, x1, x2");
}
