#ifndef LOG_SAFE_MACROS_H
#define LOG_SAFE_MACROS_H

#include <semaphore.h>
#include <commons/log.h>
#include "../utils/logger/logger.h"

// Logger thread safe, USAR ESTOS

// Cambiar a 1 para logear los debugs
// Cambiar a 0 para no logear los debugs
#define DEBUG_MODE 1

#define SOLO_LOGS_OBLIGATORIOS 0 // 1 para solo obligatorios, 0 para todos


#define LOG_OBLIGATORIO(...)                      \
    do {                                          \
        lock_logger();                            \
        log_info(get_logger(), __VA_ARGS__);      \
        unlock_logger();                          \
    } while(0)

#if SOLO_LOGS_OBLIGATORIOS
    #define LOG_INFO(...) do { } while(0)
    #define LOG_WARNING(...) do { } while(0)
    #define LOG_ERROR(...) do { } while(0)
    #define LOG_DEBUG(...) do { } while(0)
#else
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
    #define LOG_ERROR(...)                            \
        do {                                          \
            lock_logger();                            \
            log_error(get_logger(), __VA_ARGS__);     \
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
        #define LOG_DEBUG(...) do { } while(0)
    #endif
#endif

#endif
