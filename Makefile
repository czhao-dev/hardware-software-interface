# FreeRTOS Smart Task Scheduler

CC ?= gcc
TARGET := freertos_sim

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build

FREERTOS_DIR ?= FreeRTOS
FREERTOS_KERNEL_DIR ?= $(firstword $(wildcard $(FREERTOS_DIR)/Source) $(FREERTOS_DIR))
FREERTOS_PORT_DIR ?= $(firstword \
	$(wildcard $(FREERTOS_KERNEL_DIR)/portable/ThirdParty/GCC/Posix) \
	$(wildcard $(FREERTOS_KERNEL_DIR)/portable/GCC/Posix) \
	$(FREERTOS_KERNEL_DIR)/portable/ThirdParty/GCC/Posix)

CFLAGS ?= -Wall -Wextra -Wpedantic -pthread
CPPFLAGS := -I$(INC_DIR) -I$(FREERTOS_KERNEL_DIR)/include -I$(FREERTOS_PORT_DIR) -D__linux__
LDFLAGS ?= -pthread

APP_SRCS := $(wildcard $(SRC_DIR)/*.c)
FREERTOS_SRCS := \
	$(FREERTOS_PORT_DIR)/port.c \
	$(wildcard $(FREERTOS_PORT_DIR)/utils/wait_for_event.c) \
	$(FREERTOS_KERNEL_DIR)/portable/MemMang/heap_4.c \
	$(FREERTOS_KERNEL_DIR)/croutine.c \
	$(FREERTOS_KERNEL_DIR)/event_groups.c \
	$(FREERTOS_KERNEL_DIR)/list.c \
	$(FREERTOS_KERNEL_DIR)/queue.c \
	$(FREERTOS_KERNEL_DIR)/tasks.c \
	$(FREERTOS_KERNEL_DIR)/timers.c

SRCS := $(APP_SRCS) $(FREERTOS_SRCS)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean check-deps

all: check-deps $(TARGET)

check-deps:
	@if [ ! -d "$(FREERTOS_DIR)" ]; then \
		printf "Missing FreeRTOS kernel sources at %s\n" "$(FREERTOS_DIR)"; \
		printf "See docs/SETUP.md for dependency setup instructions.\n"; \
		exit 1; \
	fi
	@if [ ! -f "$(FREERTOS_PORT_DIR)/port.c" ]; then \
		printf "Missing POSIX port at %s\n" "$(FREERTOS_PORT_DIR)"; \
		printf "Set FREERTOS_PORT_DIR if your FreeRTOS checkout uses a different POSIX port path.\n"; \
		exit 1; \
	fi

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
