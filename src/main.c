/*********************************************************************
 * Entry point for FreeRTOS Smart Task Scheduler
 * ---------------------------------------------
 * Initializes queues, semaphores, and creates all tasks.
 * Starts the FreeRTOS scheduler.
 ********************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "shared_defs.h"
#include "led_task.h"
#include "temp_task.h"
#include "motion_task.h"
#include "logger_task.h"
#include "command_task.h"
#include "system_monitor.h"

QueueHandle_t ledCommandQueue;
QueueHandle_t tempCommandQueue;
QueueHandle_t loggerQueue;
SemaphoreHandle_t printMutex;

int main(void) {
    ledCommandQueue = xQueueCreate(LED_COMMAND_QUEUE_LENGTH, sizeof(LEDCommand));
    tempCommandQueue = xQueueCreate(TEMP_COMMAND_QUEUE_LENGTH, sizeof(TempCommand));
    loggerQueue = xQueueCreate(LOGGER_QUEUE_LENGTH, sizeof(char *));

    printMutex = xSemaphoreCreateMutex();

    if (ledCommandQueue == NULL || tempCommandQueue == NULL ||
        loggerQueue == NULL || printMutex == NULL) {
        return 1;
    }

    vQueueAddToRegistry(ledCommandQueue, "LED Commands");
    vQueueAddToRegistry(tempCommandQueue, "Temp Commands");
    vQueueAddToRegistry(loggerQueue, "Logger Messages");

    if (xTaskCreate(vLEDTask, "LED Task", 256, NULL, 2, NULL) != pdPASS ||
        xTaskCreate(vTemperatureTask, "Temp Task", 256, NULL, 2, NULL) != pdPASS ||
        xTaskCreate(vMotionTask, "Motion Task", 256, NULL, 2, NULL) != pdPASS ||
        xTaskCreate(vLoggerTask, "Logger", 512, NULL, 3, NULL) != pdPASS ||
        xTaskCreate(vCommandTask, "Command", 256, NULL, 1, NULL) != pdPASS ||
        xTaskCreate(vSystemMonitorTask, "Monitor", 512, NULL, 1, NULL) != pdPASS) {
        return 1;
    }

    // Start the scheduler
    vTaskStartScheduler();

    // If all is well, this line should never be reached
    for (;;);
    return 0;
}
