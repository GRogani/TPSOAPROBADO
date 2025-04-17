#ifndef UTILS_SERIALIZATION_PROTOCOL_H
#define UTILS_SERIALIZATION_PROTOCOL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../safe_alloc.h"
#include "../../macros/log_error.h"

/**
 * @enum OPCODE
 * @brief Códigos de operación para paquetes de comunicación
 * @note - `HANDSHAKE`
 */
typedef enum {
    HANDSHAKE  // Operación inicial de handshake/autenticación
} OPCODE;

/**
 * @struct t_buffer
 * @brief Buffer para serialización/deserialización de datos.
 * @note - `size` Tamaño total del buffer en bytes.
 * 
 *  - `stream` Puntero al stream de datos serializados.
 * 
 *  - `offset` Offset actual para operaciones de lectura/escritura.
 * Mantiene un stream de bytes con posición actual para lectura/escritura secuencial
 */
typedef struct t_buffer {
    uint32_t size;           // Tamaño total del buffer en bytes
    void* stream;       // Puntero al stream de datos serializados
    uint32_t offset;    // Offset actual para operaciones de lectura/escritura
} t_buffer;

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

/**
 * @brief Crea un nuevo buffer
 * @param size Tamaño inicial en bytes
 * @return t_buffer* Puntero al buffer creado, NULL en caso de error
 * 
 * @note El buffer creado debe liberarse con buffer_destroy()
 */
t_buffer* buffer_create(uint32_t size);

/**
 * @brief Destruye un buffer liberando sus recursos
 * @param buffer Puntero al buffer a destruir (puede ser NULL)
 */
void buffer_destroy(t_buffer *buffer);

/**
 * @brief Agrega datos arbitrarios al buffer
 * @param buffer Buffer destino
 * @param data Puntero a los datos a agregar
 * @param size Tamaño en bytes de los datos
 * 
 * @warning data debe apuntar a una región válida de al menos 'size' bytes
 */
void buffer_add(t_buffer *buffer, void *data, uint32_t size);

/**
 * @brief Agrega un entero de 32 bits al buffer
 * @param buffer Buffer destino
 * @param data Valor a agregar
 */
void buffer_add_uint32(t_buffer *buffer, uint32_t data);

/**
 * @brief Agrega una cadena de texto al buffer
 * @param buffer Buffer destino
 * @param length Longitud de la cadena (sin contar el null-terminator)
 * @param string Puntero a la cadena
 * 
 * @note No se almacena el null-terminator en el buffer
 */
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);

/**
 * @brief Agrega un puntero al buffer
 * @param buffer Buffer destino
 * @param ptr Puntero a almacenar
 * 
 * @warning El puntero solo será válido en el mismo espacio de direcciones
 */
void buffer_add_pointer(t_buffer *buffer, void *ptr);

/**
 * @brief Lee datos del buffer
 * @param buffer Buffer fuente
 * @param data Puntero donde almacenar los datos leídos
 * @param size Cantidad de bytes a leer
 * 
 * @pre data debe tener espacio suficiente para los bytes solicitados
 */
void buffer_read(t_buffer *buffer, void *data, uint32_t size);

/**
 * @brief Lee un entero de 32 bits del buffer
 * @param buffer Buffer fuente
 * @return uint32_t Valor leído
 */
uint32_t buffer_read_uint32(t_buffer *buffer);

/**
 * @brief Lee una cadena del buffer
 * @param buffer Buffer fuente
 * @param length Puntero donde almacenar la longitud leída
 * @return char* Cadena leída (debe liberarse manualmente)
 * 
 * @note La cadena retornada es una copia y debe liberarse con free()
 */
char *buffer_read_string(t_buffer *buffer, uint32_t *length);

/**
 * @brief Lee un puntero del buffer
 * @param buffer Buffer fuente
 * @return void* Puntero leído
 * 
 * @warning El puntero solo será válido en el mismo espacio de direcciones
 *          donde fue originalmente serializado
 */
void* buffer_read_pointer(t_buffer *buffer);

//FUNCIONES PARA CREAR Y LIBERAR PAQUETES
t_package* package_create(cod_op cod_op, t_buffer* buffer);
void* package_get_stream(t_package* package);
void stream_destroy(void* stream);
void package_destroy(t_package* package);
#endif
