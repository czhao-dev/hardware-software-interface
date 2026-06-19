"""The RV32I integer register file."""

from .bits import to_signed, to_unsigned

ABI_NAMES = [
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
]


class RegisterFile:
    """32 general-purpose registers, x0-x31. x0 is hardwired to zero."""

    def __init__(self) -> None:
        self._regs = [0] * 32

    def read(self, index: int) -> int:
        """Read register ``index`` as an unsigned 32-bit value."""
        return self._regs[index]

    def read_signed(self, index: int) -> int:
        """Read register ``index`` as a signed 32-bit value."""
        return to_signed(self._regs[index])

    def write(self, index: int, value: int) -> None:
        """Write ``value`` to register ``index``. Writes to x0 are discarded."""
        if index == 0:
            return
        self._regs[index] = to_unsigned(value)

    def snapshot(self) -> list[int]:
        """Return a copy of all 32 register values."""
        return list(self._regs)

    def name(self, index: int) -> str:
        return f"x{index}"

    def abi_name(self, index: int) -> str:
        return ABI_NAMES[index]
