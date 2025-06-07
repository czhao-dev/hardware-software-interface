/*********************************************************************************************************
 * Motion Detection Task
 *--------------------
 * This task simulates motion detection events and logs them.
 * It runs independently and generates motion detection events at random intervals.
 * It uses a mutex for printing to ensure thread safety and sends log messages to a logger queue.
 *********************************************************************************************************/

#include "motion_task.h"
#include "shared_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern SemaphoreHandle_t printMutex;
extern QueueHandle_t loggerQueue;

void vMotionTask(void *pvParameters) {
    srand((unsigned) time(NULL) + 1); // Ensure different seed than temp task

    char *logMsg;

    while (1) {
        // Simulate random delay between 3 to 10 seconds
        int delay = 3000 + rand() % 7000;
        vTaskDelay(pdMS_TO_TICKS(delay));

        // Simulate motion detection event
        xSemaphoreTake(printMutex, portMAX_DELAY);
        printf("[MOTION] Motion detected!\n");
        xSemaphoreGive(printMutex);

        // Log motion detection
        logMsg = strdup("Motion detected by sensor");
        xQueueSend(loggerQueue, &logMsg, portMAX_DELAY);
    }
}
