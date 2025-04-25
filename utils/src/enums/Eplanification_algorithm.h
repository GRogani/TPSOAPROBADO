#ifndef ENUM_PLANIFICATION_ALGORITHM
#define ENUM_PLANIFICATION_ALGORITHM

#define PLANIFICATION_ENUM_SIZE 4

#include <stddef.h>
#include <strings.h>

typedef enum {
    FIFO,
    SJF,
    SRT,
    PMCP,
    PLANIFICATION_INVALID = -1
} PLANIFICATION_ALGORITHM;


/**
 * @brief Convierte un string en su correspondiente algoritmo de planificación.
 * @param planification_algorithm Cadena de texto con el nombre del algoritmo.
 * @return Valor del enum PLANIFICATION_ALGORITHM acotado correspondiente, o PLANIFICATION_INVALID si no coincide.
 */
PLANIFICATION_ALGORITHM short_planification_from_string(char*);

/**
 * @brief Convierte un string en su correspondiente algoritmo de planificación.
 * @param planification_algorithm Cadena de texto con el nombre del algoritmo.
 * @return Valor del enum PLANIFICATION_ALGORITHM acotado correspondiente, o PLANIFICATION_INVALID si no coincide.
 */
PLANIFICATION_ALGORITHM ready_planification_from_string(char*);

#endif