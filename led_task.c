/**************************************************************************************************
 * LED Task
 * ----------------
 * This file is part of a FreeRTOS application that manages an LED's state and blinking frequency.
 * It listens for commands from a queue to turn the LED on or off and to adjust its blinking frequency.
 * It also logs actions to a logger queue.
 **************************************************************************************************/

#include "led_task.h"
#include "shared_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

// Internal LED state
static int ledState = 0;            // 0 = off, 1 = on
static int blinkFrequency = 1000;   // in milliseconds

extern SemaphoreHandle_t printMutex;
extern QueueHandle_t loggerQueue;
extern QueueHandle_t commandQueue;

void vLEDTask(void *pvParameters) {
    char *logMsg;
    TickType_t delayTicks;

    while (1) {
        delayTicks = pdMS_TO_TICKS(blinkFrequency);

        if (ledState) {
            xSemaphoreTake(printMutex, portMAX_DELAY);
            printf("[LED] Blinking\n");
            xSemaphoreGive(printMutex);
        }

        // Check if there's a command for LED
        LEDCommand cmd;
        if (xQueueReceive(ledCommandQueue, &cmd, 0)) {
            if (strcmp(cmd.type, "on") == 0) {
                ledState = 1;
                logMsg = strdup("LED turned ON");
                xQueueSend(loggerQueue, &logMsg, portMAX_DELAY);
            } else if (strcmp(cmd.type, "off") == 0) {
                ledState = 0;
                logMsg = strdup("LED turned OFF");
                xQueueSend(loggerQueue, &logMsg, portMAX_DELAY);
            } else if (strcmp(cmd.type, "freq") == 0 && cmd.value > 0) {
                blinkFrequency = cmd.value;
                char msg[64];
                snprintf(msg, sizeof(msg), "LED frequency set to %d ms", blinkFrequency);
                logMsg = strdup(msg);
                xQueueSend(loggerQueue, &logMsg, portMAX_DELAY);
            }
        }

        vTaskDelay(delayTicks);
    }
}
