"""Builds the flat-binary example programs under examples/.

Run with: python tools/build_examples.py
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

from riscv_sim import encoder as e  # noqa: E402

EXAMPLES_DIR = Path(__file__).resolve().parent.parent / "examples"


def write_program(rel_path: str, source_lines: list[str], words: list[int]) -> None:
    out_path = EXAMPLES_DIR / rel_path
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(b"".join(w.to_bytes(4, "little") for w in words))

    listing_path = out_path.with_suffix(".s")
    lines = [f"# {rel_path} -- assembly listing (for reference; not assembled by the simulator)"]
    for addr, (text, _word) in enumerate(zip(source_lines, words)):
        lines.append(f"# {addr * 4:#06x}: {text}")
    listing_path.write_text("\n".join(lines) + "\n")
    print(f"wrote {out_path.relative_to(EXAMPLES_DIR.parent)} ({len(words)} instructions)")


def build_arithmetic() -> None:
    source = [
        "addi x1, x0, 5",
        "addi x2, x0, 10",
        "add  x3, x1, x2",
        "ebreak",
    ]
    words = [
        e.addi(1, 0, 5),
        e.addi(2, 0, 10),
        e.add(3, 1, 2),
        e.ebreak(),
    ]
    write_program("arithmetic/addi.bin", source, words)


def build_branches() -> None:
    # Sums 1..5 into x2 using a countdown loop in x1.
    source = [
        "addi x1, x0, 5      # counter",
        "addi x2, x0, 0      # sum",
        "add  x2, x2, x1     # loop:",
        "addi x1, x1, -1",
        "bne  x1, x0, loop",
        "ebreak",
    ]
    words = [
        e.addi(1, 0, 5),
        e.addi(2, 0, 0),
        e.add(2, 2, 1),
        e.addi(1, 1, -1),
        e.bne(1, 0, -8),
        e.ebreak(),
    ]
    write_program("branches/loop.bin", source, words)


def build_memory() -> None:
    source = [
        "addi x1, x0, 100    # value",
        "addi x2, x0, 64     # address",
        "sw   x1, 0(x2)",
        "lw   x3, 0(x2)",
        "ebreak",
    ]
    words = [
        e.addi(1, 0, 100),
        e.addi(2, 0, 64),
        e.sw(2, 1, 0),
        e.lw(3, 2, 0),
        e.ebreak(),
    ]
    write_program("memory/load_store.bin", source, words)


if __name__ == "__main__":
    build_arithmetic()
    build_branches()
    build_memory()
