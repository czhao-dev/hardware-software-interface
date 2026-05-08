/**************************************************************************************************
 * Temperature Task
 *------------------
 * Description: This task monitors the temperature and logs warnings if it exceeds a threshold.
 * It can also receive commands to change the temperature threshold.
 **************************************************************************************************/
 
#include "temp_task.h"
#include "shared_defs.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define TEMP_SAMPLE_PERIOD_MS 3000
#define TEMP_SIM_MIN_C 20
#define TEMP_SIM_MAX_C 40

static int tempThreshold = TEMP_DEFAULT_THRESHOLD_C;

static int clamp_threshold(int threshold) {
    if (threshold < TEMP_MIN_THRESHOLD_C) {
        return TEMP_MIN_THRESHOLD_C;
    }

    if (threshold > TEMP_MAX_THRESHOLD_C) {
        return TEMP_MAX_THRESHOLD_C;
    }

    return threshold;
}

static void apply_threshold_update(const TempCommand *cmd) {
    tempThreshold = clamp_threshold(cmd->newThreshold);
    send_log(loggerQueue, portMAX_DELAY, "Temperature threshold set to %d°C", tempThreshold);
}

static void sample_temperature(uint32_t *rngState) {
    int temp = random_range(rngState, TEMP_SIM_MIN_C, TEMP_SIM_MAX_C);

    xSemaphoreTake(printMutex, portMAX_DELAY);
    printf("[TEMP] Current Temperature: %d°C (threshold: %d°C)\n", temp, tempThreshold);
    xSemaphoreGive(printMutex);

    if (temp > tempThreshold) {
        send_log(loggerQueue, portMAX_DELAY, "WARNING: High Temperature: %d°C", temp);
    }
}

void vTemperatureTask(void *pvParameters) {
    (void) pvParameters;

    uint32_t rngState = (uint32_t) time(NULL) ^ 0x54A71239U;
    const TickType_t samplePeriod = pdMS_TO_TICKS(TEMP_SAMPLE_PERIOD_MS);
    TickType_t lastSampleTick = xTaskGetTickCount() - samplePeriod;

    while (1) {
        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed = now - lastSampleTick;

        if (elapsed >= samplePeriod) {
            sample_temperature(&rngState);
            lastSampleTick = now;
            continue;
        }

        TempCommand cmd;
        if (xQueueReceive(tempCommandQueue, &cmd, samplePeriod - elapsed) == pdTRUE) {
            apply_threshold_update(&cmd);

            while (xQueueReceive(tempCommandQueue, &cmd, 0) == pdTRUE) {
                apply_threshold_update(&cmd);
            }
        }
    }
}
