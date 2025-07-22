#ifndef SERIALIZATION_PACKAGE_H
#define SERIALIZATION_PACKAGE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../macros/logger_macro.h"
#include "utils/serialization/buffer.h"
#include "utils/socket/client.h"
#include "utils/socket/server.h"
#include "enums/Eopcodes.h"

/**
 * @struct t_package
 * @brief Paquete completo de comunicación
 * @note
 * - `opcode` Tipo de operación (ej. HANDSHAKE)
 * 
 * - `buffer` Puntero al Buffer de lectura/escritura
 */
typedef struct t_package {
    OPCODE opcode;       
    t_buffer* buffer;    
} t_package;

/**
 * @brief Crea un nuevo paquete con el opcode y buffer especificado
 * @param opcode Tipo de operación
 * @param buffer puntero al buffer
 * @return Puntero al paquete creado
 */
t_package* create_package(OPCODE opcode, t_buffer* buffer);

/**
 * @brief Libera la memoria de un paquete
 * @param package Paquete a destruir
 */
void destroy_package (t_package* package);

/**
 * @brief Serializa un paquete para ser enviado por socket
 * @param package Paquete a serializar
 * @param total_size Puntero donde se guarda el tamaño total del stream serializado
 * @return Stream serializado listo para enviar
 */
void* serialize_package(t_package* package, int32_t* total_size);

/**
 * @brief Envia un paquete a través de un socket
 * @param socket Socket por el que se enviará el paquete
 * @param package Paquete a enviar
 * @return Cantidad de bytes enviados
 */
int send_package(int socket, t_package* package);

/**
 * @brief Recibe un paquete desde un socket (blocking)
 * @param socket Socket desde el cual se recibe
 * @return Paquete recibido, o NULL si hubo error
 */
t_package* recv_package(int socket);


#endif 
