/*******************************************************************************
 * Logger Task
 * -------------
 * This FreeRTOS task handles centralized logging for the system. It waits
 * on the loggerQueue for incoming log messages from other tasks (e.g.,
 * LED, temperature, motion). Once a message is received, it prints the
 * message to the console with a timestamp and then frees the allocated memory.
 *******************************************************************************/

#include "logger_task.h"
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

void vLoggerTask(void *pvParameters) {
    char *logMsg;

    while (1) {
        // Wait for a message from any task
        if (xQueueReceive(loggerQueue, &logMsg, portMAX_DELAY) == pdTRUE) {
            // Print the log message with timestamp
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            xSemaphoreTake(printMutex, portMAX_DELAY);
            printf("[LOG %02d:%02d:%02d] %s\n", t->tm_hour, t->tm_min, t->tm_sec, logMsg);
            xSemaphoreGive(printMutex);

            // Free the allocated log message
            free(logMsg);
        }
    }
}
