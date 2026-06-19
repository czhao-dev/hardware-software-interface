from riscv_sim.registers import RegisterFile


def test_initial_state_is_zero():
    regs = RegisterFile()
    assert regs.snapshot() == [0] * 32


def test_x0_is_hardwired_to_zero():
    regs = RegisterFile()
    regs.write(0, 0xDEADBEEF)
    assert regs.read(0) == 0


def test_write_and_read_round_trip():
    regs = RegisterFile()
    regs.write(5, 42)
    assert regs.read(5) == 42


def test_negative_values_wrap_to_unsigned_32_bit():
    regs = RegisterFile()
    regs.write(1, -1)
    assert regs.read(1) == 0xFFFFFFFF
    assert regs.read_signed(1) == -1


def test_abi_names():
    regs = RegisterFile()
    assert regs.abi_name(0) == "zero"
    assert regs.abi_name(2) == "sp"
    assert regs.abi_name(10) == "a0"
