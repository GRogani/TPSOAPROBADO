#ifndef ENUM_PLANIFICATION_ALGORITHM
#define ENUM_PLANIFICATION_ALGORITHM

#define PLANIFICATION_ENUM_SIZE 3

#include <stddef.h>
#include <strings.h>

typedef enum {
    FIFO,
    SJF,
    SRT,
    PLANIFICATION_INVALID = -1
} PLANIFICATION_ALGORITHM;


/**
 * @brief Convierte un string en su correspondiente algoritmo de planificación.
 * @param planification_algorithm Cadena de texto con el nombre del algoritmo.
 * @return Valor del enum PLANIFICATION_ALGORITHM correspondiente, o PLANIFICATION_INVALID si no coincide.
 */
PLANIFICATION_ALGORITHM planification_from_string(char*);

#endif