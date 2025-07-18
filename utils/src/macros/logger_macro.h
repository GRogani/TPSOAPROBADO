#ifndef LOG_SAFE_MACROS_H
#define LOG_SAFE_MACROS_H

#include <semaphore.h>
#include <commons/log.h>
#include "../utils/logger/logger.h"

// Logger thread safe, USAR ESTOS

// Cambiar a 1 para logear los debugs
// Cambiar a 0 para no logear los debugs
#define DEBUG_MODE 1

#define SOLO_LOGS_OBLIGATORIOS 1 // 1 para solo obligatorios, 0 para todos


#define LOG_OBLIGATORIO(fmt, ...)                                 \
    do {                                                         \
        lock_logger();                                           \
        char _log_obl_msg[1024];                                 \
        snprintf(_log_obl_msg, sizeof(_log_obl_msg), fmt, ##__VA_ARGS__); \
        char _log_obl_colored[1200];                             \
        snprintf(_log_obl_colored, sizeof(_log_obl_colored), "\x1b[32m%s\x1b[0m", _log_obl_msg); \
        log_info(get_logger(), "%s", _log_obl_colored);        \
        unlock_logger();                                         \
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
