import pytest

from riscv_sim import encoder as e
from riscv_sim.decoder import decode
from riscv_sim.errors import IllegalInstructionError


def test_decode_r_type_add():
    instr = decode(e.add(3, 1, 2))
    assert instr.fmt == "R"
    assert instr.mnemonic == "add"
    assert (instr.rd, instr.rs1, instr.rs2) == (3, 1, 2)


def test_decode_i_type_addi_positive_and_negative_immediate():
    pos = decode(e.addi(1, 0, 5))
    assert pos.mnemonic == "addi"
    assert (pos.rd, pos.rs1, pos.imm) == (1, 0, 5)

    neg = decode(e.addi(1, 1, -1))
    assert neg.imm == -1


def test_decode_shift_immediate_uses_shamt_field():
    instr = decode(e.slli(5, 1, 3))
    assert instr.mnemonic == "slli"
    assert instr.imm == 3


def test_decode_s_type_store():
    instr = decode(e.sw(2, 1, 0x40))
    assert instr.fmt == "S"
    assert instr.mnemonic == "sw"
    assert (instr.rs1, instr.rs2, instr.imm) == (2, 1, 0x40)


def test_decode_b_type_branch_negative_offset():
    instr = decode(e.bne(1, 0, -8))
    assert instr.fmt == "B"
    assert instr.mnemonic == "bne"
    assert instr.imm == -8


def test_decode_u_type_lui():
    instr = decode(e.lui(5, 0x12345000))
    assert instr.fmt == "U"
    assert instr.imm == 0x12345000


def test_decode_j_type_jal():
    instr = decode(e.jal(1, 0x800))
    assert instr.fmt == "J"
    assert instr.mnemonic == "jal"
    assert instr.imm == 0x800


def test_decode_system_instructions():
    assert decode(e.ecall()).mnemonic == "ecall"
    assert decode(e.ebreak()).mnemonic == "ebreak"


def test_decode_unknown_opcode_raises():
    with pytest.raises(IllegalInstructionError):
        decode(0b1111111)  # opcode bits that map to no known instruction


def test_instruction_str_formats_operands():
    assert str(decode(e.addi(1, 0, 5))) == "addi x1, x0, 5"
    assert str(decode(e.add(3, 1, 2))) == "add x3, x1, x2"
