# Makefile for FreeRTOS Smart Task Scheduler (Simulation)

# Compiler and flags
CC = gcc
CFLAGS = -I. -I./FreeRTOS/include -I./FreeRTOS/portable/GCC/Posix -Wall -Wextra -D__linux__ -pthread
LDFLAGS = -pthread

# Source files
SRCS = \
    main.c \
    led_task.c \
    temp_task.c \
    motion_task.c \
    logger_task.c \
    command_task.c \
    system_monitor.c \
    utils.c \
    FreeRTOS/portable/GCC/Posix/port.c \
    FreeRTOS/portable/MemMang/heap_4.c \
    FreeRTOS/croutine.c \
    FreeRTOS/event_groups.c \
    FreeRTOS/list.c \
    FreeRTOS/queue.c \
    FreeRTOS/tasks.c \
    FreeRTOS/timers.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output
TARGET = freertos_sim

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
