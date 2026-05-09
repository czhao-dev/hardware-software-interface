# FreeRTOS Smart Task Scheduler

A POSIX-based FreeRTOS simulation that models a small smart embedded system with multiple cooperating tasks. The project demonstrates task scheduling, queue-based message passing, mutex-protected console output, centralized logging, and lightweight runtime monitoring.

This repo is structured as a portfolio-friendly embedded systems sample: application code is separated from headers, the FreeRTOS kernel is treated as an external dependency, and the docs explain how the simulated tasks interact.

## Highlights

- **RTOS task orchestration**: LED, temperature, motion, command, logger, and system monitor tasks run concurrently.
- **Inter-task communication**: FreeRTOS queues carry typed LED commands, temperature threshold updates, and heap-owned log messages.
- **Synchronization**: A shared mutex prevents overlapping console output from multiple tasks.
- **Safer simulation helpers**: Centralized log formatting and task-local pseudo-random generators keep task behavior predictable.
- **System visibility**: The monitor task reports runtime stats and heap availability.
- **Desktop simulation**: Designed to run without physical hardware using the FreeRTOS POSIX port.

## Repository Layout

```text
.
├── include/                 # Project headers and FreeRTOSConfig.h
├── src/                     # Application task implementations
├── docs/                    # Architecture and setup notes
├── Makefile                 # POSIX simulation build
├── LICENSE
└── README.md
```

## Requirements

- GCC or Clang C compiler
- GNU Make
- POSIX threads / `pthread` support
- FreeRTOS kernel sources with the POSIX port
- Unix-like shell environment

## Task Overview

| Task | Role |
| --- | --- |
| LED Task | Receives LED commands and simulates on/off/blink-period behavior. |
| Temperature Task | Generates simulated readings and logs warnings above a configurable threshold. |
| Motion Task | Generates randomized motion events. |
| Command Task | Simulates operator commands by publishing queue messages. |
| Logger Task | Prints timestamped messages from every task and frees log allocations. |
| System Monitor | Prints runtime stats and available heap periodically. |

## Build

The FreeRTOS kernel sources are intentionally not vendored in this repo. Place or symlink a compatible FreeRTOS checkout at `./FreeRTOS`, then build:

```bash
make
./freertos_sim
```

If your kernel or POSIX port lives elsewhere, override the paths:

```bash
make FREERTOS_DIR=/path/to/FreeRTOS FREERTOS_PORT_DIR=/path/to/FreeRTOS/portable/ThirdParty/GCC/Posix
```

For dependency setup details, see [docs/SETUP.md](docs/SETUP.md). For the task communication model, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## License

Apache License 2.0. See [LICENSE](LICENSE).
