#ifndef UTILS_SWAP_IN_H
#define UTILS_SWAP_IN_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

/**
 * @brief Envía a Memoria la petición de la operación de SWAP-IN
 * Request: [ SWAP_IN, PID ]
 * @param pid proceso a desuspender
 */
int send_swap_in_package(int socket, uint32_t pid);
t_package *create_swap_in_package(uint32_t pid);
uint32_t read_swap_in_package(t_package* package); // Lectura por parte de MEMORIA

#endif