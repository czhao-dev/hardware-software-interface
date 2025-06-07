# FreeRTOS Smart Task Scheduler (Simulation)

This project simulates a smart embedded system using the FreeRTOS real-time operating system. It models a multi-tasking environment with different task types, communication mechanisms, and system monitoring features. The goal is to showcase how FreeRTOS tasks can interact using queues and semaphores, reflecting principles of concurrent programming and real-time system design.

This simulation does not require physical hardware and runs entirely on a POSIX-compliant system using the FreeRTOS POSIX port. It is designed for educational use, interview preparation, and showcasing embedded systems development skills on GitHub.

## Features

- **LED Task**: Toggles virtual LEDs based on command messages.
- **Temperature Task**: Simulates temperature readings and triggers logs if a threshold is exceeded.
- **Motion Task**: Simulates motion detection and logs status periodically.
- **Logger Task**: Centralized logging system that prints timestamped messages from all tasks.
- **Command Task**: Simulates user commands to control LED and temperature behavior.
- **System Monitor**: Periodically reports heap usage and task runtime stats.

## File Structure

```
.
├── main.c
├── led_task.c/h
├── temp_task.c/h
├── motion_task.c/h
├── logger_task.c/h
├── command_task.c/h
├── system_monitor.c/h
├── utils.c/h
├── shared_defs.h
├── FreeRTOS/        # FreeRTOS kernel and ported files (not included)
└── Makefile
```

## Requirements

- GCC (Linux/Mac) with POSIX thread support
- FreeRTOS (ported for POSIX simulation)

## Build & Run

```bash
make            # Build the project
./freertos_sim  # Run the simulation
```

To clean build artifacts:
```bash
make clean
```

## License
Apache License 2.0 — free to use and modify.
