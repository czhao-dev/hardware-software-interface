# Computer Systems Lab

A collection of systems-level projects spanning instruction set architecture,
real-time operating systems, digital hardware design, and embedded robotics.
Each project is self-contained and explores a different layer of the computing
stack — from silicon-level HDL up through real-time firmware running on
physical hardware.

## Projects

| Project | Area | Language / Tools | Description |
| --- | --- | --- | --- |
| [riscv/isa-simulator](riscv/isa-simulator) | ISA Simulation | C++17, CMake | A lightweight RV32I simulator with a CLI, instruction decoder, byte-addressable memory model, and 42-test suite for exploring processor execution at the instruction level. |
| [rtos/freertos-task-scheduler](rtos/freertos-task-scheduler) | RTOS | C, FreeRTOS, POSIX | A POSIX-based FreeRTOS simulation of a smart embedded system demonstrating concurrent task scheduling, queue-based IPC, mutex-protected I/O, and runtime heap monitoring. |
| [hardware/verilog-mini-cpu](hardware/verilog-mini-cpu) | Digital Hardware | Verilog, Python, Icarus | An 8-bit pipelined CPU with a custom 16-opcode ISA, a 2-stage IF/EX-WB pipeline, a two-pass Python assembler, and a fully self-checking testbench suite. |
| [robotics/natcar-autonomous-vehicle](robotics/natcar-autonomous-vehicle) | Embedded Robotics | C/C++, Arduino/Teensy, Eagle | An autonomous race car for the IEEE NATCAR competition: custom PCB, 128-pixel line-scan camera sensing, PD steering control, and dual PWM motor drive. |

## Highlights

**RISC-V ISA Simulator** — Implements the full RV32I base integer ISA in C++17:
all six instruction formats (R, I, S, B, U, J), a 32-register file with `x0`
hardwired to zero, little-endian memory with bounds checking, a flat binary
loader, and a `riscv-sim` CLI with full-run, traced, and single-step modes.
42 unit and integration tests cover the decoder, register file, memory model,
individual instruction semantics, and complete example-program runs.

**FreeRTOS Smart Task Scheduler** — Six tasks (LED, Temperature, Motion,
Command, Logger, System Monitor) run concurrently on the FreeRTOS POSIX port.
Typed FreeRTOS queues carry LED commands, temperature threshold updates, and
heap-owned log messages between tasks; a shared mutex prevents output
interleaving; the monitor task reports runtime stats and free heap on a
15-second cycle. Builds with zero warnings in application code on macOS and Linux.

**Verilog Mini CPU** — A complete 8-bit processor from scratch: a 16-operation
ALU, 8-register file, instruction/data memory, a 2-stage IF/EX-WB pipeline
register, and a control unit — all in synthesizable Verilog. A redirect-driven
flush mechanism handles branches and jumps with a 1-cycle penalty and no
forwarding logic. A companion two-pass Python assembler targets the custom ISA
and generates `$readmemh`-compatible hex. Verified with per-module unit tests
and an end-to-end program test (`loop_sum`: 1 + 2 + … + 10).

**IEEE NATCAR Autonomous Race Car** — Covers the full embedded design cycle:
custom PCB designed in Eagle, analog front-end circuitry for inductive guide
sensing, 128-pixel line-scan camera integration, and a PD control loop that
estimates line position, commands servo steering, and modulates motor PWM based
on turn sharpness. Built and track-tested by a four-person team across nine
firmware iterations.

## Skills and Concepts Covered

| Layer | Concepts |
| --- | --- |
| Digital Logic / RTL | Verilog, pipelined datapath, hazard analysis, ALU design, synchronous vs. asynchronous read/write |
| Computer Architecture | RISC-V RV32I, instruction formats, decoding, register files, memory models, program loading |
| Systems Programming | C/C++17, CMake, memory layout, bit manipulation, signed/unsigned arithmetic |
| Real-Time Systems | FreeRTOS task scheduling, queues, mutexes, heap management, POSIX threading |
| Embedded Hardware | PCB design (Eagle), Teensy/Arduino, servo and motor PWM, analog sensor front ends |
| Verification & Testing | Unit tests, integration tests, self-checking testbenches, waveform capture (VCD/GTKWave) |
| Toolchain | Python assembler, CMake, GNU Make, Icarus Verilog, custom test harnesses |

## Repository Structure

```
computer-systems-lab/
├── README.md
├── riscv/
│   └── isa-simulator/          # RV32I simulator (C++17)
│       ├── include/riscv_sim/  # headers: decoder, cpu, memory, etc.
│       ├── src/                # implementation + riscv-sim CLI
│       ├── tests/              # 42-test suite
│       └── examples/           # flat binary programs
├── rtos/
│   └── freertos-task-scheduler/ # FreeRTOS POSIX simulation (C)
│       ├── include/
│       ├── src/                 # task implementations
│       └── docs/                # architecture + setup notes
├── hardware/
│   └── verilog-mini-cpu/        # 8-bit pipelined CPU (Verilog)
│       ├── src/                 # RTL modules
│       ├── tb/                  # testbenches
│       ├── tools/               # Python assembler
│       └── examples/            # assembly programs
└── robotics/
    └── natcar-autonomous-vehicle/ # IEEE NATCAR race car (C/C++)
        ├── firmware/              # final + archived sketches
        ├── hardware/              # Eagle schematic and board files
        ├── docs/                  # system overview, control, calibration
        └── data/                  # inductor measurement data
```

Each project keeps its own README, build instructions, and license in its
respective directory.

## Quick Start

Each project has its own build prerequisites; see the individual READMEs for
full setup instructions. At a glance:

```bash
# RISC-V ISA Simulator (requires CMake 3.16+ and a C++17 compiler)
cd riscv/isa-simulator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/riscv_sim_tests                                # run 42 tests
./build/riscv-sim --trace examples/arithmetic/addi.bin # trace example

# FreeRTOS Task Scheduler (requires FreeRTOS kernel sources at ./FreeRTOS)
cd rtos/freertos-task-scheduler
make && ./freertos_sim

# Verilog Mini CPU (requires Icarus Verilog and Python 3)
cd hardware/verilog-mini-cpu
make test        # run all Verilog + assembler tests
make test-program  # assemble loop_sum.asm and simulate end-to-end

# NATCAR Firmware (requires Arduino IDE with Teensyduino)
# Open robotics/natcar-autonomous-vehicle/firmware/natcar_final/natcar_final.ino
```
