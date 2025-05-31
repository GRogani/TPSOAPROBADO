#ifndef LOG_SAFE_MACROS_H
#define LOG_SAFE_MACROS_H

#include <semaphore.h>
#include <commons/log.h>
#include "../utils/logger/logger.h"

// Logger thread safe, USAR ESTOS

// Cambiar a 1 para logear los debugs
// Cambiar a 0 para no logear los debugs
#define DEBUG_MODE 1


#define LOG_ERROR(...)                            \
    do {                                          \
        lock_logger();                            \
        log_error(get_logger(), __VA_ARGS__);     \
        unlock_logger();                          \
    } while(0)

#define LOG_INFO(...)                             \
    do {                                          \
        lock_logger();                            \
        log_info(get_logger(), __VA_ARGS__);      \
        unlock_logger();                          \
    } while(0)

#define LOG_WARNING(...)                          \
    do {                                          \
        lock_logger();                            \
        log_warning(get_logger(), __VA_ARGS__);   \
        unlock_logger();                          \
    } while(0)

#if DEBUG_MODE
    #define LOG_DEBUG(...)                            \
        do {                                          \
            lock_logger();                            \
            log_debug(get_logger(), __VA_ARGS__);     \
            unlock_logger();                          \
        } while(0)

#else
    #define LOG_DEBUG(...)                            \
        do {                                          \
            /* No-op in non-debug mode */             \
        } while(0)
#endif

#endif
