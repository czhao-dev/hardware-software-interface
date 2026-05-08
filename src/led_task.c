/**************************************************************************************************
 * LED Task
 * ----------------
 * This file is part of a FreeRTOS application that manages an LED's state and blinking frequency.
 * It listens for commands from a queue to turn the LED on or off and to adjust its blinking frequency.
 * It also logs actions to a logger queue.
 **************************************************************************************************/

#include "led_task.h"
#include "shared_defs.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>

// Internal LED state
static int ledState = 0;            // 0 = off, 1 = on
static int blinkPeriodMs = LED_DEFAULT_BLINK_PERIOD_MS;

static int clamp_blink_period(int periodMs) {
    if (periodMs < LED_MIN_BLINK_PERIOD_MS) {
        return LED_MIN_BLINK_PERIOD_MS;
    }

    if (periodMs > LED_MAX_BLINK_PERIOD_MS) {
        return LED_MAX_BLINK_PERIOD_MS;
    }

    return periodMs;
}

static void handle_led_command(const LEDCommand *cmd) {
    switch (cmd->type) {
        case LED_COMMAND_ON:
            ledState = 1;
            send_log(loggerQueue, portMAX_DELAY, "LED turned ON");
            break;

        case LED_COMMAND_OFF:
            ledState = 0;
            send_log(loggerQueue, portMAX_DELAY, "LED turned OFF");
            break;

        case LED_COMMAND_SET_PERIOD_MS:
            blinkPeriodMs = clamp_blink_period(cmd->value);
            send_log(loggerQueue, portMAX_DELAY, "LED blink period set to %d ms", blinkPeriodMs);
            break;

        default:
            send_log(loggerQueue, portMAX_DELAY, "Ignored unknown LED command: %d", (int) cmd->type);
            break;
    }
}

void vLEDTask(void *pvParameters) {
    (void) pvParameters;

    while (1) {
        TickType_t waitTicks = ledState ? pdMS_TO_TICKS(blinkPeriodMs) : portMAX_DELAY;
        LEDCommand cmd;

        if (xQueueReceive(ledCommandQueue, &cmd, waitTicks) == pdTRUE) {
            handle_led_command(&cmd);

            while (xQueueReceive(ledCommandQueue, &cmd, 0) == pdTRUE) {
                handle_led_command(&cmd);
            }
        } else if (ledState) {
            xSemaphoreTake(printMutex, portMAX_DELAY);
            printf("[LED] Blinking\n");
            xSemaphoreGive(printMutex);
        }
    }
}
