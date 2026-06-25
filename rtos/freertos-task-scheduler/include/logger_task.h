// logger_task.h

#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Function prototype for the logger task
void vLoggerTask(void *pvParameters);

#endif // LOGGER_TASK_H
