#include "time_utils.h"
#include <sys/time.h>
#include <stddef.h>  // For NULL

/**
 * @brief Obtiene el timestamp actual en milisegundos
 * @return uint64_t Timestamp en milisegundos
 */
uint64_t now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}
