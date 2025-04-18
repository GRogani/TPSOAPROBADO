#ifndef SERIALIZATION_PACKAGE_H
#define SERIALIZATION_PACKAGE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../safe_alloc.h"
#include "../../macros/log_error.h"
#include "enums/Eopcodes.h"

/**
 * @struct t_package
 * @brief Paquete completo de comunicación
 * @note
 * - `opcode` Tipo de operación (ej. HANDSHAKE)
 * 
 * - `buffer` Datos serializados asociados al paquete
 */
typedef struct t_package {
    OPCODE opcode;  // Tipo de operación (ej. HANDSHAKE)
    t_buffer* buffer;         // Datos serializados asociados al paquete
} t_package;

#endif