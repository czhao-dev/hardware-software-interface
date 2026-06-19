import subprocess
import sys
from pathlib import Path

from riscv_sim.cpu import Status
from riscv_sim.loader import cpu_from_binary

EXAMPLES_DIR = Path(__file__).resolve().parent.parent / "examples"


def test_arithmetic_example_program():
    cpu = cpu_from_binary(EXAMPLES_DIR / "arithmetic" / "addi.bin")
    cpu.run(max_steps=10)
    assert cpu.status == Status.HALTED
    assert cpu.regs.read(1) == 5
    assert cpu.regs.read(2) == 10
    assert cpu.regs.read(3) == 15


def test_branches_example_program():
    cpu = cpu_from_binary(EXAMPLES_DIR / "branches" / "loop.bin")
    cpu.run(max_steps=100)
    assert cpu.status == Status.HALTED
    assert cpu.regs.read(1) == 0
    assert cpu.regs.read(2) == 15  # 5 + 4 + 3 + 2 + 1


def test_memory_example_program():
    cpu = cpu_from_binary(EXAMPLES_DIR / "memory" / "load_store.bin")
    cpu.run(max_steps=10)
    assert cpu.status == Status.HALTED
    assert cpu.regs.read(1) == 100
    assert cpu.regs.read(2) == 64
    assert cpu.regs.read(3) == 100


def test_cli_trace_output_matches_readme_example():
    result = subprocess.run(
        [sys.executable, "-m", "riscv_sim.main", str(EXAMPLES_DIR / "arithmetic" / "addi.bin"), "--trace"],
        capture_output=True,
        text=True,
        check=True,
    )
    assert "pc=0x00000000  instr=0x00500093  addi x1, x0, 5" in result.stdout
    assert "pc=0x00000004  instr=0x00a00113  addi x2, x0, 10" in result.stdout
    assert "pc=0x00000008  instr=0x002081b3  add x3, x1, x2" in result.stdout
    assert "x3 = 0x0000000f" in result.stdout
    assert "halted: ebreak" in result.stdout


def test_cli_dump_registers():
    result = subprocess.run(
        [sys.executable, "-m", "riscv_sim.main", str(EXAMPLES_DIR / "memory" / "load_store.bin"), "--dump-registers"],
        capture_output=True,
        text=True,
        check=True,
    )
    assert "x1  = 0x00000064" in result.stdout
    assert "x2  = 0x00000040" in result.stdout
    assert "x3  = 0x00000064" in result.stdout
    assert result.returncode == 0
