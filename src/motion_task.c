/*********************************************************************************************************
 * Motion Detection Task
 *--------------------
 * This task simulates motion detection events and logs them.
 * It runs independently and generates motion detection events at random intervals.
 * It uses a mutex for printing to ensure thread safety and sends log messages to a logger queue.
 *********************************************************************************************************/

#include "motion_task.h"
#include "shared_defs.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define MOTION_MIN_DELAY_MS 3000
#define MOTION_MAX_DELAY_MS 10000

void vMotionTask(void *pvParameters) {
    (void) pvParameters;

    uint32_t rngState = (uint32_t) time(NULL) ^ 0x4D07104EU;

    while (1) {
        // Simulate random delay between 3 to 10 seconds
        int delay = random_range(&rngState, MOTION_MIN_DELAY_MS, MOTION_MAX_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(delay));

        // Simulate motion detection event
        xSemaphoreTake(printMutex, portMAX_DELAY);
        printf("[MOTION] Motion detected!\n");
        xSemaphoreGive(printMutex);

        // Log motion detection
        send_log(loggerQueue, portMAX_DELAY, "Motion detected by sensor");
    }
}
