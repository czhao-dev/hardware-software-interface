/********************************************************************************************
 * Command Task
 * -------------
 * This task simulates user input and sends control commands to other tasks
 * via message queues. It cycles through various commands to control LED state,
 * update temperature thresholds, and simulate system interaction without a user interface.
 ********************************************************************************************/

#include "command_task.h"
#include "shared_defs.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <time.h>

#define COMMAND_PERIOD_MS 8000
#define LED_PERIOD_MIN_MS 500
#define LED_PERIOD_MAX_MS 1250
#define LED_PERIOD_STEP_MS 250

static int random_led_period(uint32_t *rngState) {
    int steps = (LED_PERIOD_MAX_MS - LED_PERIOD_MIN_MS) / LED_PERIOD_STEP_MS;
    return LED_PERIOD_MIN_MS + (random_range(rngState, 0, steps) * LED_PERIOD_STEP_MS);
}

void vCommandTask(void *pvParameters) {
    (void) pvParameters;

    uint32_t rngState = (uint32_t) time(NULL) ^ 0xC0A1CAFEU;
    int counter = 0;

    while (1) {
        if (counter % 4 == 0 || counter % 4 == 2) {
            LEDCommand ledCmd;
            ledCmd.value = 0;

            if (counter % 4 == 0) {
                ledCmd.type = LED_COMMAND_ON;
            } else {
                ledCmd.type = LED_COMMAND_OFF;
            }

            xQueueSend(ledCommandQueue, &ledCmd, portMAX_DELAY);
            send_log(loggerQueue, portMAX_DELAY, "[CMD] Sent LED command: %s",
                     ledCmd.type == LED_COMMAND_ON ? "on" : "off");
        } else if (counter % 4 == 1) {
            LEDCommand ledCmd;
            ledCmd.type = LED_COMMAND_SET_PERIOD_MS;
            ledCmd.value = random_led_period(&rngState);
            xQueueSend(ledCommandQueue, &ledCmd, portMAX_DELAY);

            send_log(loggerQueue, portMAX_DELAY, "[CMD] Updated LED blink period to: %d ms",
                     ledCmd.value);
        } else {
            TempCommand tempCmd;
            tempCmd.newThreshold = random_range(&rngState, 25, 34);
            xQueueSend(tempCommandQueue, &tempCmd, portMAX_DELAY);

            send_log(loggerQueue, portMAX_DELAY, "[CMD] Updated temp threshold to: %d",
                     tempCmd.newThreshold);
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(COMMAND_PERIOD_MS));
    }
}
