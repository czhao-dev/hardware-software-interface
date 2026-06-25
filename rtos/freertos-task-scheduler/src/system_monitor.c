/*************************************************************************************
 * System Monitor Task
 * --------------------
 * This task periodically prints system diagnostics such as free heap memory
 * and task runtime stats. It helps monitor the health and performance of the system.
 *************************************************************************************/

#include "system_monitor.h"
#include "shared_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include <stdio.h>

void vSystemMonitorTask(void *pvParameters) {
    (void) pvParameters;

    char statsBuffer[512];

    while (1) {
        // Delay 15 seconds between diagnostics
        vTaskDelay(pdMS_TO_TICKS(15000));

        // Get system stats
        statsBuffer[0] = '\0';
        vTaskGetRunTimeStats(statsBuffer);

        xSemaphoreTake(printMutex, portMAX_DELAY);
        printf("\n[System Monitor] Task Runtime Stats:\n%s", statsBuffer);
        printf("Free Heap: %u bytes\n\n", (unsigned int)xPortGetFreeHeapSize());
        xSemaphoreGive(printMutex);
    }
}
