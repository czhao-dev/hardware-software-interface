# Embedded Systems Lab

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![RTOS: FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-orange.svg)](https://www.freertos.org/)
[![Platform: Arduino/Teensy](https://img.shields.io/badge/platform-Arduino%2FTeensy-00979D?logo=arduino&logoColor=white)](robotics/natcar-autonomous-vehicle)

A collection of systems-level projects spanning real-time operating systems
and embedded robotics. Each project is self-contained and explores a
different layer of the computing stack — from real-time task scheduling up
through firmware running on physical hardware. Read together, the two
projects trace the boundary where software's abstractions — tasks, control
loops — meet the hardware that actually executes them. See
[How the Two Projects Relate](#how-the-two-projects-relate) for the full
thread connecting them.

## Projects

| Project | Area | Language / Tools | Description |
| --- | --- | --- | --- |
| [rtos/freertos-task-scheduler](rtos/freertos-task-scheduler) | RTOS | C, FreeRTOS, POSIX | A POSIX-based FreeRTOS simulation of a smart embedded system demonstrating concurrent task scheduling, queue-based IPC, mutex-protected I/O, and runtime heap monitoring. |
| [robotics/natcar-autonomous-vehicle](robotics/natcar-autonomous-vehicle) | Embedded Robotics | C/C++, Arduino/Teensy, Eagle | An autonomous race car for the IEEE NATCAR competition: custom PCB, 128-pixel line-scan camera sensing, PD steering control, and dual PWM motor drive. |

## Repository Structure

```
embedded-systems-lab/
├── README.md
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

## How the Two Projects Relate

The two projects aren't grouped here by coincidence — each one sits at a
different point along the boundary where software's abstractions meet the
hardware that actually executes them:

1. **`freertos-task-scheduler` is the abstraction layer.** An RTOS assumes an
   instruction set and a single processor below it, and adds the layer of
   abstraction concurrent software actually needs: tasks, queues, priorities,
   and scheduling — coordinating several pieces of software sharing one
   processor, entirely in a POSIX simulation with no physical hardware in the
   loop.
2. **`natcar-autonomous-vehicle` is where software reaches back out through
   that boundary.** A PD control loop reading a real camera and writing real
   PWM signals is software touching physical hardware at the other end of the
   stack — voltages and timing, not architectural state.

There's a second thread worth naming: `freertos-task-scheduler` is
simulate-and-verify-on-a-dev-machine by design (a POSIX simulation of the RTOS
with zero application warnings), while NATCAR is the project actually deployed
against physical reality — and its firmware archive is the record of what
changed once simulation gave way to a real track, real sensors, and real
noise.

## Skills and Concepts Covered

| Layer | Concepts |
| --- | --- |
| Systems Programming | C, memory layout, bit manipulation, signed/unsigned arithmetic |
| Real-Time Systems | FreeRTOS task scheduling, queues, mutexes, heap management, POSIX threading |
| Embedded Hardware | PCB design (Eagle), Teensy/Arduino, servo and motor PWM, analog sensor front ends |
| Verification & Testing | Unit tests, integration tests, self-checking test suites |
| Toolchain | GNU Make, custom test harnesses |

## Quick Start

Each project has its own build prerequisites; see the individual READMEs for
full setup instructions. At a glance:

```bash
# FreeRTOS Task Scheduler (requires FreeRTOS kernel sources at ./FreeRTOS)
cd rtos/freertos-task-scheduler
make && ./freertos_sim

# NATCAR Firmware (requires Arduino IDE with Teensyduino)
# Open robotics/natcar-autonomous-vehicle/firmware/natcar_final/natcar_final.ino
```

## References

**FreeRTOS Task Scheduler**
- [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) — source of the RTOS kernel and POSIX simulator port used in this project.
- [FreeRTOS Reference Manual](https://www.freertos.org/Documentation/RTOS_book.html) — API reference for tasks, queues, mutexes, and timers.

**IEEE NATCAR Autonomous Vehicle**
- [TSL1401-DB Line-Scan Camera Datasheet](robotics/natcar-autonomous-vehicle/references/28317-TSL1401-DB-Manual.pdf) — timing diagrams and electrical specs for the 128-pixel optical sensor.
- [IEEE NATCAR Competition](https://ieee.ucdavis.edu/natcar/) — competition rules and track specifications.

## License

This index repo is licensed under the MIT License — see [LICENSE](LICENSE).
Each subproject carries its own license in its own directory (the FreeRTOS
task scheduler and NATCAR vehicle are each Apache License 2.0) — check the
subproject's own `LICENSE` file before reusing its code.
