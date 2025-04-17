#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <stdlib.h>
#include <commons/log.h>
#include "../../macros/log_error.h"

/**
 * @brief Inicializa el logger global del proceso.
 *
 * Crea una instancia de logger que será accesible durante toda la ejecución del proceso
 * mediante la función `get_logger`. Si ya existe un logger inicializado, se sobreescribe.
 *
 * @param log_file_name Nombre del archivo donde se registrarán los logs. `NUNCA DEJAR EN NULL`.
 * @param process_name Nombre del proceso que aparecerá en los logs como prefijo.
 * @param log_level Nivel mínimo de logs que se registrarán.
 */
void init_logger(char* log_file_name, char* process_name, t_log_level log_level);

/**
 * @brief Devuelve la instancia actual del logger.
 *
 * Accede a la instancia del logger inicializada previamente con `init_logger`.
 * Si no se inicializo devuelve NULL.
 *
 * @return Puntero al logger actual o NULL si no hay logger inicializado.
 */
t_log* get_logger();

/**
 * @brief Libera los recursos asociados al logger global.
 *
 * Destruye el logger actual y lo deja en NULL. Luego de llamar a esta función,
 * se debe volver a invocar `init_logger` para volver a usar logging.
 */
void destroy_logger();


#endif