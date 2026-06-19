# RISC-V ISA Simulator

A lightweight RISC-V instruction set simulator for learning, testing, and
experimenting with processor behavior at the instruction level.

This project is intended to model the execution of RISC-V assembly programs in
software. The simulator will provide a clear view of registers, memory,
instruction decoding, program execution, and runtime state so that users can
better understand how RISC-V programs run on a processor.

> Project status: a working RV32I core, CLI, example programs, and test suite
> are implemented in Python (see [Getting Started](#getting-started)). 42
> unit/integration tests pass, and all three example programs run correctly
> end-to-end (see [Testing Strategy](#testing-strategy)).

## Table of Contents

- [Overview](#overview)
- [Project Goals](#project-goals)
- [Implemented Features](#implemented-features)
- [RISC-V Background](#risc-v-background)
- [Simulator Design](#simulator-design)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
- [Example Usage](#example-usage)
- [Testing Strategy](#testing-strategy)
- [Development Roadmap](#development-roadmap)
- [Contributing](#contributing)
- [License](#license)

## Overview

RISC-V is an open instruction set architecture (ISA) used in embedded systems,
computer architecture education, research, and commercial processors. Unlike a
full hardware emulator, an ISA simulator focuses on the architectural behavior
of instructions: how each instruction changes registers, memory, and control
flow.

This simulator is designed to be understandable first. The implementation should
favor readable decoding, explicit execution logic, and useful diagnostics over
overly clever optimizations. That makes the project suitable for students,
computer architecture practice, debugging small RISC-V programs, and exploring
how instructions map to machine state.

## Project Goals

The main goals of this project are:

- Simulate core RISC-V instruction execution in a predictable way.
- Provide visibility into register state, memory state, and program counter
  changes.
- Support small assembly or machine-code programs for experimentation.
- Include tests that validate instruction behavior against expected results.
- Keep the codebase approachable for people learning CPU architecture.
- Build a foundation that can later support pipelining, hazards, caches, or
  other microarchitecture concepts.

## Implemented Features

The simulator currently supports:

- Full RV32I base integer instruction execution (arithmetic, logical, shifts,
  loads, stores, branches, jumps, `lui`/`auipc`, `ecall`/`ebreak`, `fence`).
- 32 general-purpose integer registers, `x0` through `x31`, with `x0` correctly
  hardwired to zero.
- Program counter tracking, including correct `jal`/`jalr` link-register and
  branch-offset behavior.
- Instruction decoding for all six RV32I formats (R, I, S, B, U, J).
- A byte-addressable, little-endian memory model with bounds checking that
  raises a clear error on invalid access.
- A program loader for flat binary machine-code images.
- A command-line interface (`riscv-sim`) with full-run, traced, and
  single-step execution modes, plus a final register dump.
- Example RV32I programs covering arithmetic, branching/looping, and
  load/store behavior.
- A 42-test unit and integration suite (decoder, registers, memory,
  instruction semantics, and full example-program runs).

Possible future extensions include:

- RV64I support.
- Multiplication and division extension support.
- Compressed instruction support.
- ELF loading.
- System call simulation.
- Pipeline visualization.
- Cache simulation.
- Command-line debugger commands.
- Web-based or graphical execution visualization.

## RISC-V Background

RISC-V instructions are encoded using a small set of instruction formats. The
base integer ISA uses 32-bit instructions and a register file containing 32
integer registers.

Common instruction formats include:

| Format | Purpose | Examples |
| --- | --- | --- |
| R-type | Register-register operations | `add`, `sub`, `and`, `or` |
| I-type | Immediate operations and loads | `addi`, `lw`, `jalr` |
| S-type | Stores | `sw`, `sh`, `sb` |
| B-type | Conditional branches | `beq`, `bne`, `blt` |
| U-type | Upper immediates | `lui`, `auipc` |
| J-type | Unconditional jumps | `jal` |

Register `x0` is hardwired to zero. Writes to `x0` should be ignored by the
simulator, matching the architectural behavior of RISC-V.

## Simulator Design

The simulator should be organized around a small number of clear components.

### CPU State

The CPU state represents the architectural state of the simulated processor:

- Program counter.
- Integer register file.
- Memory interface.
- Execution status.
- Optional counters or trace information.

### Instruction Decoder

The decoder is responsible for translating a raw 32-bit instruction into a
structured representation that the execution engine can understand.

Typical decoded fields include:

- Opcode.
- Destination register `rd`.
- Source registers `rs1` and `rs2`.
- Function fields `funct3` and `funct7`.
- Immediate value.
- Instruction format.

### Execution Engine

The execution engine applies the behavior of a decoded instruction to the CPU
state. For example:

- `add` writes `rs1 + rs2` to `rd`.
- `addi` writes `rs1 + imm` to `rd`.
- `lw` reads memory and writes the result to `rd`.
- `sw` writes a register value to memory.
- `beq` updates the program counter when two registers are equal.

### Memory Model

The memory model should provide deterministic reads and writes. A simple initial
implementation can use a byte-addressable array or map. Later versions may add
memory-mapped I/O, alignment checks, or segmented memory regions.

### Program Loader

The loader should place instructions and data into simulated memory. Early
versions may load raw machine-code words or simple text fixtures. Later versions
may support assembled binaries or ELF files.

### Diagnostics and Tracing

Useful simulator output may include:

- Current program counter.
- Instruction being executed.
- Register changes after each step.
- Memory reads and writes.
- Halt reason.
- Invalid instruction details.

## Repository Structure

```text
risc-v-isa-simulator/
|-- README.md
|-- LICENSE
|-- pyproject.toml
|-- src/
|   `-- riscv_sim/
|       |-- bits.py        # sign-extension / unsigned-wrap helpers
|       |-- registers.py   # 32-register integer file (x0 hardwired to zero)
|       |-- memory.py      # byte-addressable, little-endian memory model
|       |-- decoder.py     # raw word -> structured Instruction
|       |-- encoder.py     # mnemonic -> raw word (used by tests/examples)
|       |-- cpu.py         # fetch/decode/execute loop and CPU state
|       |-- loader.py      # loads flat binaries into memory
|       `-- main.py        # `riscv-sim` command-line interface
|-- tests/
|   |-- test_registers.py
|   |-- test_memory.py
|   |-- test_decoder.py
|   |-- test_instructions.py
|   `-- test_integration.py
|-- examples/
|   |-- arithmetic/addi.bin       (+ .s listing)
|   |-- branches/loop.bin         (+ .s listing)
|   `-- memory/load_store.bin     (+ .s listing)
`-- tools/
    `-- build_examples.py  # regenerates the example .bin files
```

## Getting Started

Requires Python 3.9+. No external runtime dependencies.

```bash
git clone https://github.com/czhao-dev/risc-v-isa-simulator.git
cd risc-v-isa-simulator

# Create a virtual environment and install the package (editable) + test deps
python3 -m venv .venv
source .venv/bin/activate
pip install -e ".[dev]"

# Run the test suite
pytest

# Run an example program
riscv-sim examples/arithmetic/addi.bin --trace --dump-registers
```

The example `.bin` files are flat binaries of raw RV32I machine code. They are
generated by `tools/build_examples.py`; rerun it with `python
tools/build_examples.py` if you change the example source in that script.

## Example Usage

```bash
# Run a sample program to completion
riscv-sim examples/arithmetic/addi.bin

# Trace every instruction and register change
riscv-sim --trace examples/arithmetic/addi.bin

# Step through a program one instruction at a time (press Enter to advance)
riscv-sim --step examples/branches/loop.bin

# Print the final register state
riscv-sim --dump-registers examples/memory/load_store.bin
```

Actual trace output for `examples/arithmetic/addi.bin`:

```text
$ riscv-sim --trace examples/arithmetic/addi.bin
pc=0x00000000  instr=0x00500093  addi x1, x0, 5
Registers changed:
x1 = 0x00000005
pc=0x00000004  instr=0x00a00113  addi x2, x0, 10
Registers changed:
x2 = 0x0000000a
pc=0x00000008  instr=0x002081b3  add x3, x1, x2
Registers changed:
x3 = 0x0000000f
pc=0x0000000c  instr=0x00100073  ebreak
halted: ebreak (exit code 0)
```

`examples/branches/loop.bin` sums 1 through 5 in a countdown loop and halts
with `x1 = 0` and `x2 = 15`. `examples/memory/load_store.bin` stores `100` to
address `64` and loads it back into `x3`, halting with `x1 = 100`, `x2 = 64`,
`x3 = 100`. Both are covered by integration tests in
[tests/test_integration.py](tests/test_integration.py).

## Testing Strategy

The simulator is tested at several levels with `pytest`:

- `tests/test_decoder.py` — decoder tests for every RV32I instruction format
  (R, I, S, B, U, J), including immediate sign-extension and unknown-opcode
  handling.
- `tests/test_instructions.py` — semantics tests for each implemented
  operation: arithmetic/logical ops, signed vs. unsigned shifts and
  comparisons, loads/stores (including sign-extended byte loads), branches,
  `jal`/`jalr` linking, `lui`/`auipc`, `ecall` exit codes, `x0` write
  suppression, and out-of-bounds memory access.
- `tests/test_registers.py` and `tests/test_memory.py` — register-file and
  memory-model unit tests.
- `tests/test_integration.py` — runs all three example programs to completion
  and checks final register state, plus CLI subprocess tests that check
  `--trace` and `--dump-registers` output.

Running `pytest` from a clean checkout currently passes all 42 tests:

```text
$ pytest
42 passed in 0.10s
```

All three example programs were also run manually end-to-end through the
`riscv-sim` CLI (`--trace`, `--step`, and `--dump-registers`) to confirm the
output matches the expected register and trace behavior described above.

## Development Roadmap

Completed milestones:

1. ✅ Project layout, language (Python), and packaging (`pyproject.toml`).
2. ✅ Register file and CPU state.
3. ✅ Byte-addressable memory model with bounds checking.
4. ✅ Instruction decoding for all RV32I formats.
5. ✅ Arithmetic and logical instruction execution.
6. ✅ Load and store instruction execution.
7. ✅ Branch and jump instruction execution.
8. ✅ Program loading from flat binaries.
9. ✅ Command-line execution (`riscv-sim`).
10. ✅ Tracing, single-step, and register-dump options.
11. ✅ Examples and automated tests.

Future milestones (see [Possible future extensions](#implemented-features) above):

12. Expand documentation with architecture notes.
13. RV64I, M-extension, compressed instructions, ELF loading, or other
    extensions, as described above.

## Contributing

Contributions are welcome. Good areas for future contributions include:

- Implementing missing instructions.
- Improving decoder coverage.
- Adding tests for edge cases.
- Writing example RISC-V programs.
- Improving trace and debug output.
- Documenting architectural behavior.
- Adding support for additional RISC-V extensions.

Before submitting a change, please make sure the relevant tests pass and that
new behavior is documented.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for
details.
