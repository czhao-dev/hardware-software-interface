/********************************************************************************************
 * Utility Functions
 * -----------------
 * This file contains helper functions used by various tasks in the system.
 * For example, memory-safe string duplication or formatting utilities.
 ********************************************************************************************/

#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Safely duplicates a string using malloc. Caller must free it.
char *safe_strdup(const char *src) {
    if (src == NULL) {
        return NULL;
    }

    char *dup = malloc(strlen(src) + 1);
    if (dup) {
        strcpy(dup, src);
    }
    return dup;
}

static char *vformat_log(const char *format, va_list args) {
    if (format == NULL) {
        return NULL;
    }

    va_list argsCopy;
    va_copy(argsCopy, args);
    int length = vsnprintf(NULL, 0, format, argsCopy);
    va_end(argsCopy);

    if (length < 0) {
        return NULL;
    }

    char *buffer = malloc((size_t) length + 1);
    if (buffer == NULL) {
        return NULL;
    }

    vsnprintf(buffer, (size_t) length + 1, format, args);
    return buffer;
}

char *format_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *message = vformat_log(format, args);
    va_end(args);

    return message;
}

BaseType_t send_log(QueueHandle_t queue, TickType_t waitTicks, const char *format, ...) {
    if (queue == NULL || format == NULL) {
        return pdFAIL;
    }

    va_list args;
    va_start(args, format);
    char *message = vformat_log(format, args);
    va_end(args);

    if (message == NULL) {
        return pdFAIL;
    }

    BaseType_t result = xQueueSend(queue, &message, waitTicks);
    if (result != pdPASS) {
        free(message);
    }

    return result;
}

uint32_t prng_next(uint32_t *state) {
    if (*state == 0U) {
        *state = 0xA5A5A5A5U;
    }

    *state = (*state * 1664525U) + 1013904223U;
    return *state;
}

int random_range(uint32_t *state, int minInclusive, int maxInclusive) {
    if (maxInclusive <= minInclusive) {
        return minInclusive;
    }

    uint32_t span = (uint32_t) (maxInclusive - minInclusive + 1);
    return minInclusive + (int) (prng_next(state) % span);
}
