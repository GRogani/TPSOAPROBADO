#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../enums/Eplanification_algorithm.h"
#include "config/t_configs.h"
#include "logger/logger.h"

/**
 * @brief Libera los recursos del kernel y destruye su logger.
 * @param kernel_config Configuración del kernel.
 * @param config Puntero a t_config que se debe liberar.
 */
void shutdown_kernel(t_kernel_config kernel_config, t_config* config);

/**
 * @brief Libera los recursos del CPU y destruye su logger.
 * @param cpu_config Configuración del CPU.
 * @param config Puntero a t_config que se debe liberar.
 */
void shutdown_cpu(t_cpu_config cpu_config, t_config* config);

/**
 * @brief Libera los recursos de memoria y destruye su logger.
 * @param memoria_config Configuración de memoria.
 * @param config Puntero a t_config que se debe liberar.
 */
void shutdown_memoria(t_memoria_config memoria_config, t_config* config);

/**
 * @brief Libera los recursos de I/O y destruye su logger.
 * @param io_config Configuración de I/O.
 * @param config Puntero a t_config que se debe liberar.
 */
void shutdown_io(t_io_config io_config, t_config* config);

#endif // SHUTDOWN_H
