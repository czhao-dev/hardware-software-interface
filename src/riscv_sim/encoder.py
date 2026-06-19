"""Encodes RV32I mnemonics into raw 32-bit instruction words.

This is a minimal assembler used to build test fixtures and example
programs; it is not a general-purpose RISC-V assembler.
"""

from .decoder import (
    OP_AUIPC, OP_BRANCH, OP_IMM, OP_JAL, OP_JALR, OP_LOAD, OP_LUI,
    OP_REG, OP_STORE, OP_SYSTEM,
)


def _u(value: int, bits: int) -> int:
    return value & ((1 << bits) - 1)


def r_type(opcode: int, rd: int, funct3: int, rs1: int, rs2: int, funct7: int) -> int:
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode


def i_type(opcode: int, rd: int, funct3: int, rs1: int, imm: int) -> int:
    return (_u(imm, 12) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode


def s_type(opcode: int, funct3: int, rs1: int, rs2: int, imm: int) -> int:
    imm = _u(imm, 12)
    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | ((imm & 0x1F) << 7) | opcode


def b_type(opcode: int, funct3: int, rs1: int, rs2: int, imm: int) -> int:
    imm = _u(imm, 13)
    bit12 = (imm >> 12) & 0x1
    bit11 = (imm >> 11) & 0x1
    bits10_5 = (imm >> 5) & 0x3F
    bits4_1 = (imm >> 1) & 0xF
    return (
        (bit12 << 31) | (bits10_5 << 25) | (rs2 << 20) | (rs1 << 15)
        | (funct3 << 12) | (bits4_1 << 8) | (bit11 << 7) | opcode
    )


def u_type(opcode: int, rd: int, imm: int) -> int:
    return (_u(imm, 32) & 0xFFFFF000) | (rd << 7) | opcode


def j_type(opcode: int, rd: int, imm: int) -> int:
    imm = _u(imm, 21)
    bit20 = (imm >> 20) & 0x1
    bits10_1 = (imm >> 1) & 0x3FF
    bit11 = (imm >> 11) & 0x1
    bits19_12 = (imm >> 12) & 0xFF
    return (bit20 << 31) | (bits10_1 << 21) | (bit11 << 20) | (bits19_12 << 12) | (rd << 7) | opcode


def add(rd, rs1, rs2): return r_type(OP_REG, rd, 0b000, rs1, rs2, 0b0000000)
def sub(rd, rs1, rs2): return r_type(OP_REG, rd, 0b000, rs1, rs2, 0b0100000)
def sll(rd, rs1, rs2): return r_type(OP_REG, rd, 0b001, rs1, rs2, 0b0000000)
def slt(rd, rs1, rs2): return r_type(OP_REG, rd, 0b010, rs1, rs2, 0b0000000)
def sltu(rd, rs1, rs2): return r_type(OP_REG, rd, 0b011, rs1, rs2, 0b0000000)
def xor(rd, rs1, rs2): return r_type(OP_REG, rd, 0b100, rs1, rs2, 0b0000000)
def srl(rd, rs1, rs2): return r_type(OP_REG, rd, 0b101, rs1, rs2, 0b0000000)
def sra(rd, rs1, rs2): return r_type(OP_REG, rd, 0b101, rs1, rs2, 0b0100000)
def or_(rd, rs1, rs2): return r_type(OP_REG, rd, 0b110, rs1, rs2, 0b0000000)
def and_(rd, rs1, rs2): return r_type(OP_REG, rd, 0b111, rs1, rs2, 0b0000000)

def addi(rd, rs1, imm): return i_type(OP_IMM, rd, 0b000, rs1, imm)
def slti(rd, rs1, imm): return i_type(OP_IMM, rd, 0b010, rs1, imm)
def sltiu(rd, rs1, imm): return i_type(OP_IMM, rd, 0b011, rs1, imm)
def xori(rd, rs1, imm): return i_type(OP_IMM, rd, 0b100, rs1, imm)
def ori(rd, rs1, imm): return i_type(OP_IMM, rd, 0b110, rs1, imm)
def andi(rd, rs1, imm): return i_type(OP_IMM, rd, 0b111, rs1, imm)
def slli(rd, rs1, shamt): return r_type(OP_IMM, rd, 0b001, rs1, shamt, 0b0000000)
def srli(rd, rs1, shamt): return r_type(OP_IMM, rd, 0b101, rs1, shamt, 0b0000000)
def srai(rd, rs1, shamt): return r_type(OP_IMM, rd, 0b101, rs1, shamt, 0b0100000)

def lb(rd, rs1, imm): return i_type(OP_LOAD, rd, 0b000, rs1, imm)
def lh(rd, rs1, imm): return i_type(OP_LOAD, rd, 0b001, rs1, imm)
def lw(rd, rs1, imm): return i_type(OP_LOAD, rd, 0b010, rs1, imm)
def lbu(rd, rs1, imm): return i_type(OP_LOAD, rd, 0b100, rs1, imm)
def lhu(rd, rs1, imm): return i_type(OP_LOAD, rd, 0b101, rs1, imm)

def sb(rs1, rs2, imm): return s_type(OP_STORE, 0b000, rs1, rs2, imm)
def sh(rs1, rs2, imm): return s_type(OP_STORE, 0b001, rs1, rs2, imm)
def sw(rs1, rs2, imm): return s_type(OP_STORE, 0b010, rs1, rs2, imm)

def beq(rs1, rs2, imm): return b_type(OP_BRANCH, 0b000, rs1, rs2, imm)
def bne(rs1, rs2, imm): return b_type(OP_BRANCH, 0b001, rs1, rs2, imm)
def blt(rs1, rs2, imm): return b_type(OP_BRANCH, 0b100, rs1, rs2, imm)
def bge(rs1, rs2, imm): return b_type(OP_BRANCH, 0b101, rs1, rs2, imm)
def bltu(rs1, rs2, imm): return b_type(OP_BRANCH, 0b110, rs1, rs2, imm)
def bgeu(rs1, rs2, imm): return b_type(OP_BRANCH, 0b111, rs1, rs2, imm)

def lui(rd, imm): return u_type(OP_LUI, rd, imm)
def auipc(rd, imm): return u_type(OP_AUIPC, rd, imm)

def jal(rd, imm): return j_type(OP_JAL, rd, imm)
def jalr(rd, rs1, imm): return i_type(OP_JALR, rd, 0b000, rs1, imm)

def ecall(): return i_type(OP_SYSTEM, 0, 0b000, 0, 0)
def ebreak(): return i_type(OP_SYSTEM, 0, 0b000, 0, 1)
