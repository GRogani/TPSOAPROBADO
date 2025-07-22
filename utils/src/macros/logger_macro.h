#ifndef LOG_SAFE_MACROS_H
#define LOG_SAFE_MACROS_H

#include <semaphore.h>
#include <commons/log.h>
#include "../utils/logger/logger.h"

#define INFO 0
#define WARNING 0
#define ERROR 0
#define DEBUG_MODE 0
#define PACKAGE 1

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
#if PACKAGE
    #define LOG_PACKAGE(...)                          \
        do {                                          \
            lock_logger();                            \
            log_info(get_logger(), __VA_ARGS__);      \
            unlock_logger();                          \
        } while(0)
#else
    #define LOG_PACKAGE(...) do { } while(0)
#endif
#if INFO
    #define LOG_INFO(...)                             \
        do {                                          \
            lock_logger();                            \
            log_info(get_logger(), __VA_ARGS__);      \
            unlock_logger();                          \
        } while(0)
#else
    #define LOG_INFO(...) do { } while(0)
#endif
#if WARNING
    #define LOG_WARNING(...)                          \
        do {                                          \
            lock_logger();                            \
            log_warning(get_logger(), __VA_ARGS__);   \
            unlock_logger();                          \
        } while(0)
#else
    #define LOG_WARNING(...) do { } while(0)
#endif
#if ERROR
    #define LOG_ERROR(...)                            \
        do {                                          \
            lock_logger();                            \
            log_error(get_logger(), __VA_ARGS__);     \
            unlock_logger();                          \
        } while(0)
#else
    #define LOG_ERROR(...) do { } while(0)
#endif
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