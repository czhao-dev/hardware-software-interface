"""Command-line interface for the RV32I simulator."""

import argparse
import sys

from .cpu import CPU, Status
from .decoder import decode
from .loader import load_binary


def _parse_args(argv: list[str] | None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(prog="riscv-sim", description="Run a RISC-V (RV32I) machine-code binary.")
    parser.add_argument("program", help="path to a flat binary file of RV32I machine code")
    parser.add_argument("--base-address", type=lambda v: int(v, 0), default=0, help="address to load the program at")
    parser.add_argument("--trace", action="store_true", help="print each instruction and its register changes")
    parser.add_argument("--step", action="store_true", help="pause for Enter before executing each instruction")
    parser.add_argument("--dump-registers", action="store_true", help="print the final register state")
    parser.add_argument("--max-steps", type=int, default=100_000, help="abort after this many instructions")
    return parser.parse_args(argv)


def _format_registers_changed(before: list[int], after: list[int]) -> str:
    lines = [f"x{i} = 0x{value:08x}" for i, (b, value) in enumerate(zip(before, after)) if b != value]
    return "\n".join(lines)


def _dump_registers(cpu: CPU) -> str:
    lines = [f"pc = 0x{cpu.pc:08x}"]
    for i in range(32):
        lines.append(f"x{i:<2} = 0x{cpu.regs.read(i):08x}")
    return "\n".join(lines)


def run(argv: list[str] | None = None) -> int:
    args = _parse_args(argv)
    memory = load_binary(args.program, args.base_address)
    cpu = CPU(memory=memory, pc=args.base_address)

    while cpu.status == Status.RUNNING:
        if cpu.step_count >= args.max_steps:
            cpu.status = Status.ERROR
            cpu.halt_reason = f"exceeded max_steps ({args.max_steps}); possible infinite loop"
            break

        pc_before = cpu.pc
        raw = cpu.memory.read_word(pc_before)
        instr = decode(raw)

        if args.step:
            input(f"pc=0x{pc_before:08x}  instr=0x{raw:08x}  {instr}  [Enter to step] ")
        elif args.trace:
            print(f"pc=0x{pc_before:08x}  instr=0x{raw:08x}  {instr}")

        before = cpu.regs.snapshot()
        try:
            cpu.step()
        except Exception as exc:  # noqa: BLE001 - reported as a simulator halt below
            print(f"error: {exc}", file=sys.stderr)
            break
        after = cpu.regs.snapshot()

        if args.trace or args.step:
            changed = _format_registers_changed(before, after)
            if changed:
                print("Registers changed:")
                print(changed)

    if cpu.halt_reason:
        print(f"halted: {cpu.halt_reason} (exit code {cpu.exit_code})")

    if args.dump_registers:
        print(_dump_registers(cpu))

    return 0 if cpu.status == Status.HALTED else 1


def main() -> None:
    sys.exit(run())


if __name__ == "__main__":
    main()
