"""The CPU execution engine: applies decoded instructions to machine state."""

import enum

from .bits import to_signed, to_unsigned
from .decoder import Instruction, decode
from .errors import IllegalInstructionError, MemoryAccessError, SimulatorError
from .memory import Memory
from .registers import RegisterFile


class Status(enum.Enum):
    RUNNING = "running"
    HALTED = "halted"
    ERROR = "error"


class CPU:
    """RV32I CPU state and execution engine."""

    def __init__(self, memory: Memory | None = None, pc: int = 0) -> None:
        self.memory = memory if memory is not None else Memory()
        self.regs = RegisterFile()
        self.pc = pc
        self.status = Status.RUNNING
        self.halt_reason: str | None = None
        self.exit_code = 0
        self.step_count = 0

    def step(self) -> Instruction:
        """Fetch, decode, and execute a single instruction.

        Returns the decoded instruction. If a fault occurs, ``status`` is set
        to ``Status.ERROR`` and ``halt_reason`` is populated; the caller
        should stop stepping once ``status`` is no longer ``RUNNING``.
        """
        try:
            raw = self.memory.read_word(self.pc)
            instr = decode(raw)
            self._execute(instr)
            self.step_count += 1
            return instr
        except SimulatorError as exc:
            self.status = Status.ERROR
            self.halt_reason = str(exc)
            raise

    def run(self, max_steps: int | None = 100_000, on_step=None) -> None:
        """Run until halted, faulted, or ``max_steps`` is reached."""
        steps = 0
        while self.status == Status.RUNNING:
            if max_steps is not None and steps >= max_steps:
                self.status = Status.ERROR
                self.halt_reason = f"exceeded max_steps ({max_steps}); possible infinite loop"
                break
            instr = self.step()
            if on_step is not None:
                on_step(instr)
            steps += 1

    def _execute(self, instr: Instruction) -> None:
        pc_next = self.pc + 4
        rs1 = self.regs.read(instr.rs1)
        rs2 = self.regs.read(instr.rs2)
        rs1_s = self.regs.read_signed(instr.rs1)
        rs2_s = self.regs.read_signed(instr.rs2)
        m = instr.mnemonic

        if m == "add":
            self.regs.write(instr.rd, rs1 + rs2)
        elif m == "sub":
            self.regs.write(instr.rd, rs1 - rs2)
        elif m == "sll":
            self.regs.write(instr.rd, rs1 << (rs2 & 0x1F))
        elif m == "slt":
            self.regs.write(instr.rd, int(rs1_s < rs2_s))
        elif m == "sltu":
            self.regs.write(instr.rd, int(rs1 < rs2))
        elif m == "xor":
            self.regs.write(instr.rd, rs1 ^ rs2)
        elif m == "srl":
            self.regs.write(instr.rd, rs1 >> (rs2 & 0x1F))
        elif m == "sra":
            self.regs.write(instr.rd, to_unsigned(rs1_s >> (rs2 & 0x1F)))
        elif m == "or":
            self.regs.write(instr.rd, rs1 | rs2)
        elif m == "and":
            self.regs.write(instr.rd, rs1 & rs2)

        elif m == "addi":
            self.regs.write(instr.rd, rs1 + instr.imm)
        elif m == "slti":
            self.regs.write(instr.rd, int(rs1_s < instr.imm))
        elif m == "sltiu":
            self.regs.write(instr.rd, int(rs1 < to_unsigned(instr.imm)))
        elif m == "xori":
            self.regs.write(instr.rd, rs1 ^ to_unsigned(instr.imm))
        elif m == "ori":
            self.regs.write(instr.rd, rs1 | to_unsigned(instr.imm))
        elif m == "andi":
            self.regs.write(instr.rd, rs1 & to_unsigned(instr.imm))
        elif m == "slli":
            self.regs.write(instr.rd, rs1 << instr.imm)
        elif m == "srli":
            self.regs.write(instr.rd, rs1 >> instr.imm)
        elif m == "srai":
            self.regs.write(instr.rd, to_unsigned(rs1_s >> instr.imm))

        elif m in ("lb", "lh", "lw", "lbu", "lhu"):
            address = to_unsigned(rs1 + instr.imm)
            if m == "lb":
                value = self.memory.read_byte(address, signed=True)
            elif m == "lh":
                value = self.memory.read_half(address, signed=True)
            elif m == "lw":
                value = self.memory.read_word(address)
            elif m == "lbu":
                value = self.memory.read_byte(address, signed=False)
            else:
                value = self.memory.read_half(address, signed=False)
            self.regs.write(instr.rd, value)

        elif m in ("sb", "sh", "sw"):
            address = to_unsigned(rs1 + instr.imm)
            if m == "sb":
                self.memory.write_byte(address, rs2)
            elif m == "sh":
                self.memory.write_half(address, rs2)
            else:
                self.memory.write_word(address, rs2)

        elif m in ("beq", "bne", "blt", "bge", "bltu", "bgeu"):
            taken = (
                (m == "beq" and rs1 == rs2)
                or (m == "bne" and rs1 != rs2)
                or (m == "blt" and rs1_s < rs2_s)
                or (m == "bge" and rs1_s >= rs2_s)
                or (m == "bltu" and rs1 < rs2)
                or (m == "bgeu" and rs1 >= rs2)
            )
            if taken:
                pc_next = to_unsigned(self.pc + instr.imm)

        elif m == "lui":
            self.regs.write(instr.rd, instr.imm)
        elif m == "auipc":
            self.regs.write(instr.rd, to_unsigned(self.pc + instr.imm))

        elif m == "jal":
            self.regs.write(instr.rd, pc_next)
            pc_next = to_unsigned(self.pc + instr.imm)
        elif m == "jalr":
            self.regs.write(instr.rd, pc_next)
            pc_next = to_unsigned((rs1 + instr.imm) & ~0x1)

        elif m == "fence":
            pass
        elif m == "ecall":
            self.status = Status.HALTED
            self.halt_reason = "ecall"
            self.exit_code = to_signed(self.regs.read(10))
        elif m == "ebreak":
            self.status = Status.HALTED
            self.halt_reason = "ebreak"
            self.exit_code = to_signed(self.regs.read(10))
        else:
            raise IllegalInstructionError(f"unimplemented mnemonic '{m}' (0x{instr.raw:08x})")

        self.pc = pc_next
