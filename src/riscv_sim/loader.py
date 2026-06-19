"""Loads program images (raw machine code) into simulator memory."""

from pathlib import Path

from .cpu import CPU
from .memory import Memory


def load_binary(path: str | Path, base_address: int = 0, memory: Memory | None = None) -> Memory:
    """Load a flat binary file of machine code into memory at ``base_address``."""
    data = Path(path).read_bytes()
    memory = memory if memory is not None else Memory()
    memory.load_bytes(base_address, data)
    return memory


def load_words(words: list[int], base_address: int = 0, memory: Memory | None = None) -> Memory:
    """Load a list of 32-bit instruction words into memory at ``base_address``."""
    memory = memory if memory is not None else Memory()
    data = b"".join(word.to_bytes(4, "little") for word in words)
    memory.load_bytes(base_address, data)
    return memory


def cpu_from_binary(path: str | Path, base_address: int = 0) -> CPU:
    """Build a ready-to-run CPU with a binary program loaded at ``base_address``."""
    memory = load_binary(path, base_address)
    return CPU(memory=memory, pc=base_address)
