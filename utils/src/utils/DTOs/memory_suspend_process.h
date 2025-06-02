#ifndef UTILS_MEMORY_SUSPEND_PROCESS_H
#define UTILS_MEMORY_SUSPEND_PROCESS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

// REQUEST

/**
 * @brief Envía a Memoria la petición de la operación de SWAP
 * Request: [ SUSPEND_PROCESS, PID ]
 * @param pid proceso a ser swapeado
 */
int send_memory_suspend_process(int socket, uint32_t pid);
t_package *create_memory_create_process(uint32_t pid);
uint32_t read_memory_suspend_process(t_package* package); // Lectura por parte de MEMORIA

// RESPONSE
/**
 * @brief Envía al Kernel (MEDIUM SCHEDULER) la respuesta de la operación de SWAP
 * Response: [ SUSPEND_PROCESS, éxito (1) o fallo (0) ]
 * @param success true (1) si el SWAP se hizo correctamente; false (0) si falló.
 */
int send_memory_suspend_process_response(int socket, uint32_t success);
t_package* create_memory_suspend_process_response(uint32_t success);
bool read_memory_suspend_process_response(t_package *package); // LECTURA por parte del Kernel

#endif