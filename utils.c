/********************************************************************************************
 * Utility Functions
 * -----------------
 * This file contains helper functions used by various tasks in the system.
 * For example, memory-safe string duplication or formatting utilities.
 ********************************************************************************************/

#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Safely duplicates a string using malloc. Caller must free it.
char* safe_strdup(const char* src) {
    if (src == NULL) return NULL;
    char* dup = malloc(strlen(src) + 1);
    if (dup) {
        strcpy(dup, src);
    }
    return dup;
}

// Format and duplicate a string safely using malloc
char* format_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return safe_strdup(buffer);
}
