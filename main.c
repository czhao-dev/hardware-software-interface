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

// Queue and semaphore handles
define QUEUE_LENGTH 5
QueueHandle_t ledCommandQueue;
QueueHandle_t tempCommandQueue;
QueueHandle_t loggerQueue;
SemaphoreHandle_t printMutex;

int main(void) {
    // Create queues
    ledCommandQueue = xQueueCreate(QUEUE_LENGTH, sizeof(LEDCommand));
    tempCommandQueue = xQueueCreate(QUEUE_LENGTH, sizeof(TempCommand));
    loggerQueue = xQueueCreate(QUEUE_LENGTH, sizeof(char*));

    // Create binary semaphore for printing
    printMutex = xSemaphoreCreateMutex();

    // Create tasks
    xTaskCreate(vLEDTask, "LED Task", 256, NULL, 2, NULL);
    xTaskCreate(vTempTask, "Temp Task", 256, NULL, 2, NULL);
    xTaskCreate(vMotionTask, "Motion Task", 256, NULL, 2, NULL);
    xTaskCreate(vLoggerTask, "Logger", 512, NULL, 3, NULL);
    xTaskCreate(vCommandTask, "Command", 256, NULL, 1, NULL);
    xTaskCreate(vSystemMonitorTask, "Monitor", 512, NULL, 1, NULL);

    // Start the scheduler
    vTaskStartScheduler();

    // If all is well, this line should never be reached
    for (;;);
    return 0;
}
