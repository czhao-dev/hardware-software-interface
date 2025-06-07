/**************************************************************************************************
 * Temperature Task
 *------------------
 * Description: This task monitors the temperature and logs warnings if it exceeds a threshold.
 * It can also receive commands to change the temperature threshold.
 **************************************************************************************************/
 
#include "temp_task.h"
#include "shared_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_TEMP_THRESHOLD 30

extern SemaphoreHandle_t printMutex;
extern QueueHandle_t loggerQueue;
extern QueueHandle_t tempCommandQueue;

static int tempThreshold = DEFAULT_TEMP_THRESHOLD;

void vTemperatureTask(void *pvParameters) {
    srand((unsigned) time(NULL));

    char *logMsg;
    int temp;

    while (1) {
        // Simulate temperature reading between 20°C and 40°C
        temp = 20 + rand() % 21;

        // Print temperature reading
        xSemaphoreTake(printMutex, portMAX_DELAY);
        printf("[TEMP] Current Temperature: %d°C\n", temp);
        xSemaphoreGive(printMutex);

        // Check against threshold
        if (temp > tempThreshold) {
            char msg[64];
            snprintf(msg, sizeof(msg), "WARNING: High Temperature: %d°C", temp);
            logMsg = strdup(msg);
            xQueueSend(loggerQueue, &logMsg, portMAX_DELAY);
        }

        // Check for threshold update command
        TempCommand cmd;
        if (xQueueReceive(tempCommandQueue, &cmd, 0)) {
            if (cmd.newThreshold > 0) {
                tempThreshold = cmd.newThreshold;
                char msg[64];
                snprintf(msg, sizeof(msg), "Temperature threshold set to %d°C", tempThreshold);
                logMsg = strdup(msg);
                xQueueSend(loggerQueue, &logMsg, portMAX_DELAY);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); // Read every 3 seconds
    }
}
