#ifndef KERNEL_MEMORY_CLIENT_H
#define KERNEL_MEMORY_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../utils.h"
#include "repository/pcb/pcb.h"


/**
 * @brief Establece conexión con el módulo de memoria
 * @param config Configuración del kernel con IP y puerto de memoria
 * @return Socket de conexión o -1 si hay error
 */
int connect_to_memory(t_kernel_config* config);

/**
 * @brief Crea un proceso en memoria
 * @param memory_socket Socket de conexión con memoria
 * @param pid PID del proceso a crear
 * @param size Tamaño del proceso en memoria
 * @param pseudocode_path Ruta al archivo de pseudocódigo
 * @return true si la creación fue exitosa, false caso contrario
 */
bool create_process(int memory_socket, int32_t pid, int32_t size, char* pseudocode_path);

/**
 * @brief Consulta espacio libre en memoria
 * @param memory_socket Socket de conexión con memoria
 * @return Cantidad de espacio libre en bytes, o 0 si hay error
 */
int32_t get_memory_free_space(int memory_socket);

/**
 * @brief Envia mensaje a memoria para eliminar el recurso mediante una coneccion efimera,
 * espera una confirmacion por parte de memoria para terminar.
 * @param pid PID del proceso a eliminar
 * @return - `0 (OK)` 
 *  
 * - `-1 (ERROR)`
 */
int kill_process_in_memory(int32_t pid);


/**
 * @brief Crea una coneccion con la memoria y enviua un paquete DUMP_MEMORY y espera una confimacion.
 * 
 * @return - `0 (OK)`
 * 
 * - `-1 (ERROR)` 
 */
bool dump_memory_routine(int32_t pid);

/**
 * @brief Desconecta de memoria
 * @param memory_socket Socket de conexión
 */
void disconnect_from_memory(int memory_socket);

#endif
