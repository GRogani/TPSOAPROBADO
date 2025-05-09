#ifndef DATA_REQUEST_H
#define DATA_REQUEST_H

#include <stdint.h>
#include <stdlib.h>

/**
 * @struct t_request_IO
 * @brief Estructura DATA de una request para facilitar los recv del Kernel
 */
typedef struct t_request_IO {
    uint32_t pid;          // Proceso que espera un evento de I/O
    uint32_t sleep_time;   // Tiempo que tarda la operación de I/O
} t_request_IO;

/**
 * @brief Libera la memoria de un t_request_IO
 * @param t_request_IO request a destruir
 */
void request_destroy(t_request_IO*);

#endif