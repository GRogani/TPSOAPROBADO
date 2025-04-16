#ifndef UTILS_MACROS_LOG_ERROR_H
#define UTILS_MACROS_LOG_ERROR_H

#include <stdio.h>

#define LOG_DEBUG_MODE false

#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "\x1b[31m[ERROR]\x1b[0m (%s:%d in %s) " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    fprintf(stderr, "\x1b[33m[WARNING]\x1b[0m (%s:%d in %s) " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    fprintf(stderr, "\x1b[34m[DEBUG]\x1b[0m (%s:%d in %s) " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#endif 