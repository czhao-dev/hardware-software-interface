# Computer Systems Lab

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/17)
[![RTOS: FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-orange.svg)](https://www.freertos.org/)
[![Platform: Arduino/Teensy](https://img.shields.io/badge/platform-Arduino%2FTeensy-00979D?logo=arduino&logoColor=white)](robotics/natcar-autonomous-vehicle)

A collection of systems-level projects spanning instruction set architecture,
real-time operating systems, and embedded robotics. Each project is
self-contained and explores a different layer of the computing stack — from
architectural simulation up through real-time firmware running on physical
hardware. Read together, the three projects trace the boundary this repo is
named for: the point where software's abstractions — instructions, tasks,
control loops — meet the hardware that actually executes them. See
[How the Three Projects Relate](#how-the-three-projects-relate) for the full
thread connecting them.

## Projects

| Project | Area | Language / Tools | Description |
| --- | --- | --- | --- |
| [riscv/isa-simulator](riscv/isa-simulator) | ISA Simulation | C++17, CMake | A lightweight RV32I simulator with a CLI, instruction decoder, byte-addressable memory model, and 42-test suite for exploring processor execution at the instruction level. |
| [rtos/freertos-task-scheduler](rtos/freertos-task-scheduler) | RTOS | C, FreeRTOS, POSIX | A POSIX-based FreeRTOS simulation of a smart embedded system demonstrating concurrent task scheduling, queue-based IPC, mutex-protected I/O, and runtime heap monitoring. |
| [robotics/natcar-autonomous-vehicle](robotics/natcar-autonomous-vehicle) | Embedded Robotics | C/C++, Arduino/Teensy, Eagle | An autonomous race car for the IEEE NATCAR competition: custom PCB, 128-pixel line-scan camera sensing, PD steering control, and dual PWM motor drive. |

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
└── robotics/
    └── natcar-autonomous-vehicle/ # IEEE NATCAR race car (C/C++)
        ├── firmware/              # final + archived sketches
        ├── hardware/              # Eagle schematic and board files
        ├── docs/                  # system overview, control, calibration
        └── data/                  # inductor measurement data
```

Each project keeps its own README, build instructions, and license in its
respective directory.

## Project Deep Dives

### RISC-V ISA Simulator ([`riscv/isa-simulator`](riscv/isa-simulator))

The simulator implements the full RV32I base integer ISA — arithmetic,
logical, shifts, loads, stores, branches, jumps, `lui`/`auipc`, and
`ecall`/`ebreak` — as pure architectural state transitions in C++17, with no
notion of gates, clocks, or pipeline stages at all. That's a deliberate choice,
not a limitation: an ISA simulator's job is to pin down *what* each
instruction must do to registers, memory, and the program counter,
independently of *how* any particular piece of hardware realizes it. The
decoder handles all six RV32I instruction formats (R/I/S/B/U/J) with explicit,
readable field extraction rather than table-driven cleverness, `x0` is
hardwired to zero at the register-file level (writes are silently dropped,
matching real RISC-V behavior), and the byte-addressable little-endian memory
model raises a clear, typed error on out-of-bounds access instead of silently
corrupting state — a debuggability property real hardware doesn't give you for
free. The `riscv-sim` CLI's three execution modes (full-run, `--trace`,
`--step`) and final register dump exist so a user can watch architectural
state evolve instruction-by-instruction, which is the entire pedagogical point
of building a simulator instead of just reading the spec. 42 unit and
integration tests validate the decoder against every instruction format, each
instruction's semantics individually (including signed/unsigned edge cases in
shifts and comparisons), and three example programs run end-to-end through the
CLI itself, not just through internal APIs.

### FreeRTOS Smart Task Scheduler ([`rtos/freertos-task-scheduler`](rtos/freertos-task-scheduler))

Six FreeRTOS tasks run concurrently on the POSIX simulator port, and the
priority assignments encode a real scheduling decision, not an arbitrary
default: the Logger task runs at priority 3 (highest), LED/Temperature/Motion
at priority 2, and Command/System-Monitor at priority 1. The logger owns every
heap-allocated log message once it's successfully enqueued and frees it after
printing — giving it the highest priority keeps that queue draining promptly
instead of letting heap-owned messages pile up behind lower-priority
producers, which would otherwise be a slow, silent heap leak under load. Three
typed queues (`ledCommandQueue`, `tempCommandQueue`, `loggerQueue`) carry
structured messages between tasks rather than raw bytes, and a single
`printMutex` serializes console output so six tasks writing concurrently never
interleave mid-line. A subtler design detail worth calling out: each
simulated-sensor task keeps its own task-local pseudo-random generator state
rather than sharing one global `rand()` call — a small thing, but it's exactly
the kind of shared-mutable-state bug that's easy to introduce and hard to
diagnose once several tasks are actually running concurrently on a real
scheduler instead of a single-threaded mental model.

### IEEE NATCAR Autonomous Race Car ([`robotics/natcar-autonomous-vehicle`](robotics/natcar-autonomous-vehicle))

![NATCAR system architecture](robotics/natcar-autonomous-vehicle/images/system-architecture.png)

NATCAR is the one project in this repo that isn't simulated or bench-tested in
isolation — it's a custom Eagle-designed PCB, a Teensy 3.1-compatible
controller, and a 128-pixel TSL1401 line-scan camera, built and tuned against
a physical track by a four-person team. The line-position estimator is a
compact edge detector: rather than thresholding the entire 128-pixel frame, it
finds the strongest rising and falling brightness transitions and averages
their positions, which is cheap enough to run every 10 ms loop iteration and
robust to lighting variation across a track. Steering is closed-loop PD
control on that position error; speed is reduced proportionally to how sharp
the turn is, and independently ramp-limited in both directions — one step per
loop coming back up after the line is reacquired, so the car doesn't snap from
a stop straight to top speed, and a 25-consecutive-lost-frame fail-safe cuts
both motors instead of driving blind. What makes the project worth reading
past the final firmware is the archive: nine iterations
(`firmware/archive/natcar_v1.1.ino` through `v1.9`) show the team starting
with three-way inductive-sensor fusion alongside the camera and progressively
simplifying down to the leaner, more reliable camera-only control loop that
shipped — a real record of an engineering team converging on what actually
worked on the track, not just the polished end state.

## How the Three Projects Relate

The three projects aren't grouped here by coincidence — each one sits at a
different point along the boundary this repo is named for, the point where
software's abstractions meet the hardware that actually executes them:

1. **`isa-simulator` pins down the contract.** RV32I's architectural
   semantics — what `add` or `beq` must do to registers, memory, and the
   program counter — are pinned down independently of any specific gate-level
   realization. An ISA is the interface both hardware designers and software
   toolchains build against, and this project models that interface directly
   rather than any one implementation of it.
2. **`freertos-task-scheduler` builds on top of that contract.** An RTOS
   assumes the ISA-level guarantees below it hold, and adds the next layer of
   abstraction software actually needs: concurrent tasks, queues, priorities,
   and scheduling — coordinating several pieces of software sharing one
   processor, the problem that doesn't exist yet at the single-instruction
   level the simulator models.
3. **`natcar-autonomous-vehicle` is where software reaches back out through
   that boundary.** A PD control loop reading a real camera and writing real
   PWM signals is software touching physical hardware at the other end of the
   stack — voltages and timing, not architectural state.

There's a second thread worth naming: two of the three projects are
simulate-and-verify-on-a-dev-machine by design (42 automated tests for the ISA
simulator, a POSIX simulation of the RTOS with zero application warnings),
while NATCAR is the one project actually deployed against physical reality —
and its firmware archive is the record of what changed once simulation gave
way to a real track, real sensors, and real noise.

## Skills and Concepts Covered

| Layer | Concepts |
| --- | --- |
| Computer Architecture | RISC-V RV32I, instruction formats, decoding, register files, memory models, program loading |
| Systems Programming | C/C++17, CMake, memory layout, bit manipulation, signed/unsigned arithmetic |
| Real-Time Systems | FreeRTOS task scheduling, queues, mutexes, heap management, POSIX threading |
| Embedded Hardware | PCB design (Eagle), Teensy/Arduino, servo and motor PWM, analog sensor front ends |
| Verification & Testing | Unit tests, integration tests, self-checking test suites |
| Toolchain | CMake, GNU Make, custom test harnesses |

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

# NATCAR Firmware (requires Arduino IDE with Teensyduino)
# Open robotics/natcar-autonomous-vehicle/firmware/natcar_final/natcar_final.ino
```

## References

**RISC-V ISA Simulator**
- [RISC-V ISA Specification, Volume I: Unprivileged ISA](https://github.com/riscv/riscv-isa-manual/releases/latest) — the authoritative reference for RV32I instruction encodings and semantics.

**FreeRTOS Task Scheduler**
- [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) — source of the RTOS kernel and POSIX simulator port used in this project.
- [FreeRTOS Reference Manual](https://www.freertos.org/Documentation/RTOS_book.html) — API reference for tasks, queues, mutexes, and timers.

**IEEE NATCAR Autonomous Vehicle**
- [TSL1401-DB Line-Scan Camera Datasheet](robotics/natcar-autonomous-vehicle/references/28317-TSL1401-DB-Manual.pdf) — timing diagrams and electrical specs for the 128-pixel optical sensor.
- [IEEE NATCAR Competition](https://ieee.ucdavis.edu/natcar/) — competition rules and track specifications.

## License

This index repo is licensed under the MIT License — see [LICENSE](LICENSE).
Each subproject carries its own license in its own directory (the RISC-V ISA
simulator is MIT; the FreeRTOS task scheduler and NATCAR vehicle are each
Apache License 2.0) — check the subproject's own `LICENSE` file before
reusing its code.
