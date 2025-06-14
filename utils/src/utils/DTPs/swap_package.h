#ifndef UTILS_SWAP_H
#define UTILS_SWAP_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

/**
 * @brief Envía a Memoria la petición de la operación de SWAP
 * Request: [ SWAP, PID ]
 * @param pid proceso a ser swapeado
 */
int send_memory_suspend_process(int socket, uint32_t pid);
t_package *create_memory_suspend_process(uint32_t pid);
uint32_t read_memory_suspend_process(t_package* package); // Lectura por parte de MEMORIA

#endif