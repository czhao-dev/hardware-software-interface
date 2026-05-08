// temp_task.h

#ifndef TEMP_TASK_H
#define TEMP_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Function prototype for the temperature monitoring task
void vTemperatureTask(void *pvParameters);

#endif // TEMP_TASK_H
