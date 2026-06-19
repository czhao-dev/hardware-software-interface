"""Decodes raw 32-bit RV32I instruction words into a structured form."""

from dataclasses import dataclass

from .bits import sign_extend
from .errors import IllegalInstructionError

# Opcode (bits [6:0]) values for each instruction format/group.
OP_LUI = 0b0110111
OP_AUIPC = 0b0010111
OP_JAL = 0b1101111
OP_JALR = 0b1100111
OP_BRANCH = 0b1100011
OP_LOAD = 0b0000011
OP_STORE = 0b0100011
OP_IMM = 0b0010011
OP_REG = 0b0110011
OP_FENCE = 0b0001111
OP_SYSTEM = 0b1110011

_BRANCH_MNEMONICS = {
    0b000: "beq", 0b001: "bne", 0b100: "blt",
    0b101: "bge", 0b110: "bltu", 0b111: "bgeu",
}
_LOAD_MNEMONICS = {
    0b000: "lb", 0b001: "lh", 0b010: "lw", 0b100: "lbu", 0b101: "lhu",
}
_STORE_MNEMONICS = {0b000: "sb", 0b001: "sh", 0b010: "sw"}
_IMM_MNEMONICS = {
    0b000: "addi", 0b010: "slti", 0b011: "sltiu", 0b100: "xori",
    0b110: "ori", 0b111: "andi", 0b001: "slli", 0b101: "srli",
}
_REG_MNEMONICS = {
    (0b000, 0b0000000): "add", (0b000, 0b0100000): "sub",
    (0b001, 0b0000000): "sll", (0b010, 0b0000000): "slt",
    (0b011, 0b0000000): "sltu", (0b100, 0b0000000): "xor",
    (0b101, 0b0000000): "srl", (0b101, 0b0100000): "sra",
    (0b110, 0b0000000): "or", (0b111, 0b0000000): "and",
}


@dataclass(frozen=True)
class Instruction:
    """A decoded RV32I instruction."""

    raw: int
    fmt: str
    mnemonic: str
    opcode: int
    rd: int = 0
    rs1: int = 0
    rs2: int = 0
    funct3: int = 0
    funct7: int = 0
    imm: int = 0

    def __str__(self) -> str:
        rd, rs1, rs2, imm = f"x{self.rd}", f"x{self.rs1}", f"x{self.rs2}", self.imm
        if self.fmt == "R":
            return f"{self.mnemonic} {rd}, {rs1}, {rs2}"
        if self.fmt == "I":
            if self.mnemonic in ("jalr", "lb", "lh", "lw", "lbu", "lhu"):
                return f"{self.mnemonic} {rd}, {imm}({rs1})"
            if self.mnemonic in ("slli", "srli", "srai"):
                return f"{self.mnemonic} {rd}, {rs1}, {imm}"
            return f"{self.mnemonic} {rd}, {rs1}, {imm}"
        if self.fmt == "S":
            return f"{self.mnemonic} {rs2}, {imm}({rs1})"
        if self.fmt == "B":
            return f"{self.mnemonic} {rs1}, {rs2}, {imm}"
        if self.fmt == "U":
            return f"{self.mnemonic} {rd}, {imm}"
        if self.fmt == "J":
            return f"{self.mnemonic} {rd}, {imm}"
        if self.fmt == "SYSTEM":
            return self.mnemonic
        return f"{self.mnemonic}"


def decode(raw: int) -> Instruction:
    """Decode a 32-bit instruction word."""
    opcode = raw & 0x7F
    rd = (raw >> 7) & 0x1F
    funct3 = (raw >> 12) & 0x7
    rs1 = (raw >> 15) & 0x1F
    rs2 = (raw >> 20) & 0x1F
    funct7 = (raw >> 25) & 0x7F

    if opcode == OP_LUI:
        imm = sign_extend(raw & 0xFFFFF000, 32)
        return Instruction(raw, "U", "lui", opcode, rd=rd, imm=imm)

    if opcode == OP_AUIPC:
        imm = sign_extend(raw & 0xFFFFF000, 32)
        return Instruction(raw, "U", "auipc", opcode, rd=rd, imm=imm)

    if opcode == OP_JAL:
        imm = sign_extend(
            ((raw >> 21) & 0x3FF) << 1
            | ((raw >> 20) & 0x1) << 11
            | ((raw >> 12) & 0xFF) << 12
            | ((raw >> 31) & 0x1) << 20,
            21,
        )
        return Instruction(raw, "J", "jal", opcode, rd=rd, imm=imm)

    if opcode == OP_JALR:
        imm = sign_extend(raw >> 20, 12)
        return Instruction(raw, "I", "jalr", opcode, rd=rd, rs1=rs1, funct3=funct3, imm=imm)

    if opcode == OP_BRANCH:
        mnemonic = _BRANCH_MNEMONICS.get(funct3)
        if mnemonic is None:
            raise IllegalInstructionError(f"unknown branch funct3=0b{funct3:03b} in 0x{raw:08x}")
        imm = sign_extend(
            ((raw >> 8) & 0xF) << 1
            | ((raw >> 25) & 0x3F) << 5
            | ((raw >> 7) & 0x1) << 11
            | ((raw >> 31) & 0x1) << 12,
            13,
        )
        return Instruction(raw, "B", mnemonic, opcode, rs1=rs1, rs2=rs2, funct3=funct3, imm=imm)

    if opcode == OP_LOAD:
        mnemonic = _LOAD_MNEMONICS.get(funct3)
        if mnemonic is None:
            raise IllegalInstructionError(f"unknown load funct3=0b{funct3:03b} in 0x{raw:08x}")
        imm = sign_extend(raw >> 20, 12)
        return Instruction(raw, "I", mnemonic, opcode, rd=rd, rs1=rs1, funct3=funct3, imm=imm)

    if opcode == OP_STORE:
        mnemonic = _STORE_MNEMONICS.get(funct3)
        if mnemonic is None:
            raise IllegalInstructionError(f"unknown store funct3=0b{funct3:03b} in 0x{raw:08x}")
        imm = sign_extend(((raw >> 7) & 0x1F) | ((raw >> 25) & 0x7F) << 5, 12)
        return Instruction(raw, "S", mnemonic, opcode, rs1=rs1, rs2=rs2, funct3=funct3, imm=imm)

    if opcode == OP_IMM:
        mnemonic = _IMM_MNEMONICS.get(funct3)
        if mnemonic is None:
            raise IllegalInstructionError(f"unknown OP-IMM funct3=0b{funct3:03b} in 0x{raw:08x}")
        if funct3 == 0b101:
            mnemonic = "srai" if funct7 == 0b0100000 else "srli"
        if mnemonic in ("slli", "srli", "srai"):
            imm = rs2  # shift amount lives in the rs2 field (imm[4:0])
        else:
            imm = sign_extend(raw >> 20, 12)
        return Instruction(raw, "I", mnemonic, opcode, rd=rd, rs1=rs1, funct3=funct3, funct7=funct7, imm=imm)

    if opcode == OP_REG:
        mnemonic = _REG_MNEMONICS.get((funct3, funct7))
        if mnemonic is None:
            raise IllegalInstructionError(
                f"unknown OP funct3=0b{funct3:03b} funct7=0b{funct7:07b} in 0x{raw:08x}"
            )
        return Instruction(raw, "R", mnemonic, opcode, rd=rd, rs1=rs1, rs2=rs2, funct3=funct3, funct7=funct7)

    if opcode == OP_FENCE:
        return Instruction(raw, "FENCE", "fence", opcode)

    if opcode == OP_SYSTEM:
        imm = raw >> 20
        mnemonic = "ebreak" if imm == 1 else "ecall"
        return Instruction(raw, "SYSTEM", mnemonic, opcode, imm=imm)

    raise IllegalInstructionError(f"unknown opcode 0b{opcode:07b} in 0x{raw:08x}")
