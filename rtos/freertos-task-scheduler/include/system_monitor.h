// system_monitor.h

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "FreeRTOS.h"
#include "task.h"

// Function prototype for the system monitor task
void vSystemMonitorTask(void *pvParameters);

#endif // SYSTEM_MONITOR_H
