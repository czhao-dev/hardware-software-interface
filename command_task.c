/********************************************************************************************
 * Command Task
 * -------------
 * This task simulates user input and sends control commands to other tasks
 * via message queues. It cycles through various commands to control LED state,
 * update temperature thresholds, and simulate system interaction without a user interface.
 ********************************************************************************************/

#include "command_task.h"
#include "shared_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern QueueHandle_t ledCommandQueue;
extern QueueHandle_t tempCommandQueue;
extern SemaphoreHandle_t printMutex;
extern QueueHandle_t loggerQueue;

void vCommandTask(void *pvParameters) {
    srand((unsigned) time(NULL) + 2); // Unique seed
    int counter = 0;

    while (1) {
        if (counter % 2 == 0) {
            // Toggle LED
            LEDCommand ledCmd;
            strcpy(ledCmd.command, (counter % 4 == 0) ? "on" : "off");
            xQueueSend(ledCommandQueue, &ledCmd, portMAX_DELAY);

            char *log = malloc(64);
            snprintf(log, 64, "[CMD] Sent LED command: %s", ledCmd.command);
            xQueueSend(loggerQueue, &log, portMAX_DELAY);
        } else {
            // Update temperature threshold
            TempCommand tempCmd;
            tempCmd.newThreshold = 25 + rand() % 10;
            xQueueSend(tempCommandQueue, &tempCmd, portMAX_DELAY);

            char *log = malloc(64);
            snprintf(log, 64, "[CMD] Updated temp threshold to: %d", tempCmd.newThreshold);
            xQueueSend(loggerQueue, &log, portMAX_DELAY);
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(8000)); // Issue command every 8 seconds
    }
}
