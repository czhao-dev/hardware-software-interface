#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define LED_COMMAND_QUEUE_LENGTH 8
#define TEMP_COMMAND_QUEUE_LENGTH 4
#define LOGGER_QUEUE_LENGTH 16

#define LED_DEFAULT_BLINK_PERIOD_MS 1000
#define LED_MIN_BLINK_PERIOD_MS 100
#define LED_MAX_BLINK_PERIOD_MS 5000

#define TEMP_DEFAULT_THRESHOLD_C 30
#define TEMP_MIN_THRESHOLD_C 15
#define TEMP_MAX_THRESHOLD_C 60

typedef enum {
    LED_COMMAND_ON = 0,
    LED_COMMAND_OFF,
    LED_COMMAND_SET_PERIOD_MS
} LEDCommandType;

typedef struct {
    LEDCommandType type;
    int value;  // Used by LED_COMMAND_SET_PERIOD_MS as the blink period.
} LEDCommand;

typedef struct {
    int newThreshold;  // e.g., 30 degrees Celsius
} TempCommand;

extern QueueHandle_t ledCommandQueue;
extern QueueHandle_t tempCommandQueue;
extern QueueHandle_t loggerQueue;
extern SemaphoreHandle_t printMutex;

#endif // SHARED_DEFS_H
