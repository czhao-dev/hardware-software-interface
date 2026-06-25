# Setup

## Requirements

- GCC or Clang
- POSIX threads
- A FreeRTOS kernel checkout with the POSIX port

## FreeRTOS Dependency

This application expects the FreeRTOS kernel sources to be available at `./FreeRTOS` by default. The kernel is not committed here so the repository stays focused on the application code.

One common setup is:

```bash
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS
```

The `Makefile` supports both the standalone `FreeRTOS-Kernel` layout and the older full `FreeRTOS/Source` layout. If your checkout has a different POSIX port location, pass the paths to `make`:

```bash
make FREERTOS_DIR=/path/to/FreeRTOS FREERTOS_PORT_DIR=/path/to/FreeRTOS/portable/ThirdParty/GCC/Posix
```

## Build And Run

```bash
make
./freertos_sim
```

Clean generated files:

```bash
make clean
```

## Notes

- `include/FreeRTOSConfig.h` enables queues, mutexes, timers, heap usage, and runtime stats for the simulation.
- `make check-deps` validates that the expected FreeRTOS paths exist before compiling.
- On macOS, the POSIX FreeRTOS port may need small upstream-specific path adjustments depending on the kernel version you use.
