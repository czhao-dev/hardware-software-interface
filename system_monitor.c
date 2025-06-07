/*************************************************************************************
 * System Monitor Task
 * --------------------
 * This task periodically prints system diagnostics such as free heap memory
 * and task runtime stats. It helps monitor the health and performance of the system.
 *************************************************************************************/

#include "system_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include <stdio.h>

extern SemaphoreHandle_t printMutex;

void vSystemMonitorTask(void *pvParameters) {
    char statsBuffer[512];

    while (1) {
        // Delay 15 seconds between diagnostics
        vTaskDelay(pdMS_TO_TICKS(15000));

        // Get system stats
        vTaskGetRunTimeStats(statsBuffer);

        xSemaphoreTake(printMutex, portMAX_DELAY);
        printf("\n[System Monitor] Task Runtime Stats:\n%s", statsBuffer);
        printf("Free Heap: %u bytes\n\n", (unsigned int)xPortGetFreeHeapSize());
        xSemaphoreGive(printMutex);
    }
}
