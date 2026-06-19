"""Bit-manipulation helpers shared across the simulator."""

MASK32 = 0xFFFFFFFF


def to_unsigned(value: int) -> int:
    """Wrap an arbitrary int into the unsigned 32-bit range."""
    return value & MASK32


def to_signed(value: int) -> int:
    """Reinterpret an unsigned 32-bit value as a signed 32-bit int."""
    value &= MASK32
    return value - (1 << 32) if value & (1 << 31) else value


def sign_extend(value: int, bits: int) -> int:
    """Sign-extend ``value`` (an unsigned int with ``bits`` significant bits)."""
    mask = 1 << (bits - 1)
    value &= (1 << bits) - 1
    return (value ^ mask) - mask
