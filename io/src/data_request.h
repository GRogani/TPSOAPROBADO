#ifndef DATA_REQUEST_H
#define DATA_REQUEST_H

#include <stdint.h>
#include <stdlib.h>
#include "utils/logger/logger.h"

/**
 * @struct t_request_IO
 * @brief Estructura DATA de una request para facilitar los recv del Kernel
 */
typedef struct t_request_IO {
    int kernel_socket;     // Socket a mandar la respuesta
    char* device_name;     // Nombre del dispositivo que está realizando la operación
    uint32_t pid;          // Proceso que espera un evento de I/O
    uint32_t sleep_time;   // Tiempo que tarda la operación de I/O
} t_request_IO;
// NOTA: en realidad, en el request también está el response que es el socket y el device_name.


/**
 * @brief Libera la memoria de un t_request_IO
 * @param t_request_IO request a destruir
 */
void request_destroy(t_request_IO*);

#endif