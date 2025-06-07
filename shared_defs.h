// shared_defs.h

#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

// Structures used for inter-task communication

typedef struct {
    char command[16];  // e.g., "on" or "off"
} LEDCommand;

typedef struct {
    int newThreshold;  // e.g., 30°C
} TempCommand;

// Global queues and semaphores shared between tasks
extern QueueHandle_t ledCommandQueue;
extern QueueHandle_t tempCommandQueue;
extern QueueHandle_t loggerQueue;

extern SemaphoreHandle_t printMutex;

#endif // SHARED_DEFS_H
