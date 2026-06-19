import pytest

from riscv_sim.errors import MemoryAccessError
from riscv_sim.memory import Memory


def test_word_read_write_round_trip():
    mem = Memory(size=256)
    mem.write_word(0, 0x12345678)
    assert mem.read_word(0) == 0x12345678


def test_little_endian_byte_order():
    mem = Memory(size=256)
    mem.write_word(0, 0x12345678)
    assert mem.read_bytes(0, 4) == bytes([0x78, 0x56, 0x34, 0x12])


def test_byte_and_half_round_trip():
    mem = Memory(size=256)
    mem.write_byte(4, 0xFF)
    assert mem.read_byte(4, signed=False) == 0xFF
    assert mem.read_byte(4, signed=True) == -1

    mem.write_half(8, 0x8000)
    assert mem.read_half(8, signed=False) == 0x8000
    assert mem.read_half(8, signed=True) == -32768


def test_out_of_bounds_access_raises():
    mem = Memory(size=16)
    with pytest.raises(MemoryAccessError):
        mem.read_word(16)
    with pytest.raises(MemoryAccessError):
        mem.write_byte(-1, 0)


def test_load_bytes_places_program_image():
    mem = Memory(size=64)
    mem.load_bytes(0, b"\x01\x02\x03\x04")
    assert mem.read_word(0) == 0x04030201
