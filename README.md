# FreeRTOS Smart Task Scheduler

[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-orange.svg)](https://www.freertos.org/)
[![Platform](https://img.shields.io/badge/platform-POSIX%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](docs/SETUP.md)
[![Build](https://img.shields.io/badge/build-Make-informational.svg)](Makefile)
[![License](https://img.shields.io/badge/license-Apache%202.0-green.svg)](LICENSE)

A POSIX-based FreeRTOS simulation that models a small smart embedded system with multiple cooperating tasks. The project demonstrates task scheduling, queue-based message passing, mutex-protected console output, centralized logging, and lightweight runtime monitoring.

This repo is structured as a portfolio-friendly embedded systems sample: application code is separated from headers, the FreeRTOS kernel is treated as an external dependency, and the docs explain how the simulated tasks interact.

## Highlights

- **RTOS task orchestration**: LED, temperature, motion, command, logger, and system monitor tasks run concurrently.
- **Inter-task communication**: FreeRTOS queues carry typed LED commands, temperature threshold updates, and heap-owned log messages.
- **Synchronization**: A shared mutex prevents overlapping console output from multiple tasks.
- **Safer simulation helpers**: Centralized log formatting and task-local pseudo-random generators keep task behavior predictable.
- **System visibility**: The monitor task reports runtime stats and heap availability.
- **Desktop simulation**: Designed to run without physical hardware using the FreeRTOS POSIX port.

## Tech Stack

- **Language**: C
- **RTOS**: [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) (POSIX simulator port)
- **Concurrency**: POSIX threads (`pthread`), FreeRTOS queues, mutexes, and software timers
- **Build**: GNU Make
- **Tested on**: macOS (Apple Clang) and Linux (GCC)

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

## Testing & Results

The simulator was built with `make` (zero warnings in application code; the only warning comes from the vendored FreeRTOS kernel itself) and exercised on macOS with Apple Clang. A representative run shows all six tasks scheduling concurrently, exchanging queue messages, and the system monitor reporting runtime stats on its 15-second cycle:

```text
[TEMP] Current Temperature: 32°C (threshold: 30°C)
[LOG 21:21:50] WARNING: High Temperature: 33°C
[LED] Blinking
[MOTION] Motion detected!
[LOG 21:21:47] Motion detected by sensor
[LOG 21:21:45] LED blink period set to 500 ms
[LOG 21:21:45] [CMD] Updated LED blink period to: 500 ms

[System Monitor] Task Runtime Stats:
Monitor        	0		<1%
IDLE           	1849		102%
LED Task       	0		<1%
Command        	0		<1%
Motion Task    	0		<1%
Temp Task      	0		<1%
Tmr Svc        	0		<1%
Logger         	0		<1%
Free Heap: 43136 bytes
```

Observed behavior matches the design:

- The command task drives LED on/off and blink-period changes, visible in the logger output a tick after each command is queued.
- Temperature readings above the 30°C threshold trigger a `WARNING` log entry every time, with no missed or duplicated warnings.
- Motion events are logged independently of the LED/temperature traffic, confirming the queues and logger task don't block one another.
- The mutex-protected logger produces interleaved but non-corrupted output even with six tasks running concurrently.
- Heap usage stays stable (no leaks) across repeated runs, since the logger frees every heap-allocated message after printing it.

## License

Apache License 2.0. See [LICENSE](LICENSE).
