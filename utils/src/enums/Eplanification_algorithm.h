#ifndef ENUM_PLANIFICATION_ALGORITHM
#define ENUM_PLANIFICATION_ALGORITHM

#include <stddef.h>
#include <string.h>

typedef enum {
    FIFO,
    SJF, // con o sin desalojo
    PMCP,
} PLANIFICATION_ALGORITHM;


/**
 * @brief Convierte un string en su correspondiente algoritmo de planificación.
 * @param algorithm_str Cadena de texto con el nombre del algoritmo.
 * @return `PLANIFICATION_ALGORITHM` , `FIFO` en caso de error.
 */
PLANIFICATION_ALGORITHM planification_algorithm_from_string(const char *algorithm_str);

#endif