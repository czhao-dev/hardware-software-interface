import pytest

from riscv_sim import encoder as e
from riscv_sim.cpu import CPU, Status
from riscv_sim.errors import MemoryAccessError
from riscv_sim.loader import load_words


def cpu_with(words, base_address=0):
    memory = load_words(words, base_address)
    return CPU(memory=memory, pc=base_address)


def test_addi_and_add():
    cpu = cpu_with([e.addi(1, 0, 5), e.addi(2, 0, 10), e.add(3, 1, 2)])
    cpu.step()
    cpu.step()
    cpu.step()
    assert cpu.regs.read(1) == 5
    assert cpu.regs.read(2) == 10
    assert cpu.regs.read(3) == 15
    assert cpu.pc == 12


def test_sub_underflow_wraps_to_unsigned():
    cpu = cpu_with([e.addi(1, 0, 1), e.addi(2, 0, 2), e.sub(3, 1, 2)])
    cpu.run(max_steps=3)
    assert cpu.regs.read(3) == 0xFFFFFFFF
    assert cpu.regs.read_signed(3) == -1


@pytest.mark.parametrize(
    "encoder_name, a, b, expected",
    [
        ("and_", 0b1100, 0b1010, 0b1000),
        ("or_", 0b1100, 0b1010, 0b1110),
        ("xor", 0b1100, 0b1010, 0b0110),
    ],
)
def test_logical_r_type(encoder_name, a, b, expected):
    op = getattr(e, encoder_name)
    cpu = cpu_with([e.addi(1, 0, a), e.addi(2, 0, b), op(3, 1, 2)])
    cpu.run(max_steps=3)
    assert cpu.regs.read(3) == expected


def test_shifts():
    cpu = cpu_with([e.addi(1, 0, 1), e.slli(2, 1, 4), e.srli(3, 2, 2)])
    cpu.run(max_steps=3)
    assert cpu.regs.read(2) == 0x10
    assert cpu.regs.read(3) == 0x4


def test_sra_preserves_sign():
    cpu = cpu_with([e.addi(1, 0, -8), e.srai(2, 1, 1)])
    cpu.run(max_steps=2)
    assert cpu.regs.read_signed(2) == -4


def test_slt_signed_vs_sltu_unsigned():
    cpu = cpu_with([e.addi(1, 0, -1), e.addi(2, 0, 1), e.slt(3, 1, 2), e.sltu(4, 1, 2)])
    cpu.run(max_steps=4)
    assert cpu.regs.read(3) == 1  # -1 < 1 (signed)
    assert cpu.regs.read(4) == 0  # 0xFFFFFFFF is not < 1 (unsigned)


def test_store_and_load_word():
    cpu = cpu_with([e.addi(1, 0, 100), e.addi(2, 0, 64), e.sw(2, 1, 0), e.lw(3, 2, 0)])
    cpu.run(max_steps=4)
    assert cpu.regs.read(3) == 100


def test_load_byte_sign_extension():
    cpu = cpu_with([e.addi(1, 0, -1), e.addi(2, 0, 64), e.sb(2, 1, 0), e.lb(3, 2, 0), e.lbu(4, 2, 0)])
    cpu.run(max_steps=5)
    assert cpu.regs.read_signed(3) == -1
    assert cpu.regs.read(4) == 0xFF


def test_invalid_memory_access_raises():
    # lui sets x1 to a huge address that is well beyond the default 1 MiB memory.
    cpu = cpu_with([e.lui(1, 0x7FFFF000), e.lw(2, 1, 0)], base_address=0)
    cpu.step()
    with pytest.raises(MemoryAccessError):
        cpu.step()
    assert cpu.status == Status.ERROR


def test_branch_taken_and_not_taken():
    # bne(x1, x0, +8) skips an addi when x1 != 0.
    cpu = cpu_with([e.addi(1, 0, 1), e.bne(1, 0, 8), e.addi(2, 0, 99), e.addi(3, 0, 42)])
    cpu.run(max_steps=3)
    assert cpu.regs.read(2) == 0  # skipped
    assert cpu.regs.read(3) == 42


def test_loop_sums_one_through_five():
    words = [
        e.addi(1, 0, 5),
        e.addi(2, 0, 0),
        e.add(2, 2, 1),
        e.addi(1, 1, -1),
        e.bne(1, 0, -8),
        e.ebreak(),
    ]
    cpu = cpu_with(words)
    cpu.run(max_steps=100)
    assert cpu.status == Status.HALTED
    assert cpu.regs.read(2) == 15
    assert cpu.regs.read(1) == 0


def test_jal_and_jalr():
    # jal x1, +8 jumps over one addi and links return address in x1.
    cpu = cpu_with([e.jal(1, 8), e.addi(2, 0, 99), e.addi(3, 0, 42)])
    cpu.run(max_steps=2)
    assert cpu.regs.read(1) == 4
    assert cpu.regs.read(2) == 0
    assert cpu.regs.read(3) == 42


def test_lui_and_auipc():
    cpu = cpu_with([e.lui(1, 0x12345000), e.auipc(2, 0x1000)], base_address=0x100)
    cpu.run(max_steps=2)
    assert cpu.regs.read(1) == 0x12345000
    assert cpu.regs.read(2) == 0x100 + 0x1000 + 4


def test_ecall_halts_with_exit_code_from_a0():
    cpu = cpu_with([e.addi(10, 0, 7), e.ecall()])
    cpu.run(max_steps=2)
    assert cpu.status == Status.HALTED
    assert cpu.halt_reason == "ecall"
    assert cpu.exit_code == 7


def test_x0_writes_are_discarded():
    cpu = cpu_with([e.addi(0, 0, 123)])
    cpu.run(max_steps=1)
    assert cpu.regs.read(0) == 0
