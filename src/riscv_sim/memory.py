"""A simple byte-addressable, little-endian memory model."""

from .errors import MemoryAccessError


class Memory:
    """Fixed-size byte-addressable memory with little-endian word access."""

    def __init__(self, size: int = 1 << 20) -> None:
        self.size = size
        self._data = bytearray(size)

    def _check_range(self, address: int, length: int) -> None:
        if address < 0 or address + length > self.size:
            raise MemoryAccessError(
                f"address 0x{address:08x} (length {length}) is out of bounds "
                f"for memory of size {self.size} bytes"
            )

    def read_bytes(self, address: int, length: int) -> bytes:
        self._check_range(address, length)
        return bytes(self._data[address:address + length])

    def write_bytes(self, address: int, data: bytes) -> None:
        self._check_range(address, len(data))
        self._data[address:address + len(data)] = data

    def read_byte(self, address: int, signed: bool = False) -> int:
        value = self.read_bytes(address, 1)[0]
        return value - 256 if signed and value & 0x80 else value

    def read_half(self, address: int, signed: bool = False) -> int:
        value = int.from_bytes(self.read_bytes(address, 2), "little")
        return value - 65536 if signed and value & 0x8000 else value

    def read_word(self, address: int) -> int:
        return int.from_bytes(self.read_bytes(address, 4), "little")

    def write_byte(self, address: int, value: int) -> None:
        self.write_bytes(address, (value & 0xFF).to_bytes(1, "little"))

    def write_half(self, address: int, value: int) -> None:
        self.write_bytes(address, (value & 0xFFFF).to_bytes(2, "little"))

    def write_word(self, address: int, value: int) -> None:
        self.write_bytes(address, (value & 0xFFFFFFFF).to_bytes(4, "little"))

    def load_bytes(self, address: int, data: bytes) -> None:
        """Load a block of raw bytes (e.g. a program image) into memory."""
        self.write_bytes(address, data)
