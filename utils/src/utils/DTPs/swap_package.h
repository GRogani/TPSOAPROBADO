#ifndef UTILS_SWAP_H
#define UTILS_SWAP_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

// Firma de la función genérica que maneja SWAP-OUT / SWAP-IN
int send_swap_operation_package(int socket, uint32_t pid, OPCODE operation);

// Wrappers
int send_swap_package(int socket, uint32_t pid);
int send_swap_in_package(int socket, uint32_t pid);

/**
 * @brief Envía a Memoria la petición de una operación de SWAP genérica
 * Request: [ OPERATION_CODE, PID ]
 * @param socket socket de conexión con memoria
 * @param pid proceso a ser swapeado/deswapeado
 * @param operation código de operación (SWAP o UNSUSPEND_PROCESS)
 * @return bytes enviados
 */
int send_swap_operation_package(int socket, uint32_t pid, OPCODE operation);

/**
 * @brief Crea un paquete para operaciones de SWAP genéricas
 * @param pid proceso a ser swapeado/deswapeado
 * @param operation código de operación (SWAP o UNSUSPEND_PROCESS)
 * @return paquete creado
 */
t_package *create_swap_operation_package(uint32_t pid, OPCODE operation);

/**
 * @brief Lee un paquete de operación de SWAP (para uso en Memoria)
 * @param package paquete a leer
 * @return PID del proceso
 */
uint32_t read_swap_package(t_package* package);

#endif