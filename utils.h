// utils.h

#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>

// Duplicates a string safely (caller must free)
char* safe_strdup(const char* src);

// Formats a log message and returns a dynamically allocated string
char* format_log(const char* format, ...);

#endif // UTILS_H
