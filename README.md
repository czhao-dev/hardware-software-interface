# Computer Systems Lab

A collection of systems-level projects spanning instruction set architecture,
real-time operating systems, digital hardware design, and embedded robotics.

## Projects

| Project | Area | Description |
| --- | --- | --- |
| [riscv/isa-simulator](riscv/isa-simulator) | ISA Simulation | A lightweight RV32I instruction set simulator with a CLI, decoder, and test suite for exploring processor execution at the instruction level. |
| [rtos/freertos-task-scheduler](rtos/freertos-task-scheduler) | RTOS | A POSIX-based FreeRTOS simulation of a smart embedded system, demonstrating task scheduling, queues, mutexes, and runtime monitoring. |
| [hardware/verilog-mini-cpu](hardware/verilog-mini-cpu) | Digital Hardware | An 8-bit pipelined CPU built in Verilog with a custom ISA, a 2-stage pipeline, a Python assembler, and a self-checking testbench suite. |
| [robotics/natcar-autonomous-vehicle](robotics/natcar-autonomous-vehicle) | Embedded Robotics | An autonomous race car for the IEEE NATCAR competition, covering PCB design, line-scan camera sensing, and real-time steering/motor control. |

## Structure

```
computer-systems-lab/
├── README.md
├── riscv/
│   └── isa-simulator/
├── rtos/
│   └── freertos-task-scheduler/
├── hardware/
│   └── verilog-mini-cpu/
└── robotics/
    └── natcar-autonomous-vehicle/
```

Each project keeps its own README, license, and build instructions in its
respective directory.
