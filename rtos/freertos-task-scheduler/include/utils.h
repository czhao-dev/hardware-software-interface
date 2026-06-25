// utils.h

#ifndef UTILS_H
#define UTILS_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

// Duplicates a string safely (caller must free)
char *safe_strdup(const char *src);

// Formats a log message and returns a dynamically allocated string
char *format_log(const char *format, ...);

// Formats and sends a dynamically allocated log message. The logger owns it on success.
BaseType_t send_log(QueueHandle_t queue, TickType_t waitTicks, const char *format, ...);

// Small deterministic pseudo-random helper for task-local simulation state.
uint32_t prng_next(uint32_t *state);
int random_range(uint32_t *state, int minInclusive, int maxInclusive);

#endif // UTILS_H
