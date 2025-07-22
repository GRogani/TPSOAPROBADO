#ifndef MEMORY_WRITE_REQUEST_H
#define MEMORY_WRITE_REQUEST_H

#include <stdint.h>
#include "utils/serialization/buffer.h"
#include "utils/serialization/package.h"

// Estructura para la solicitud de escritura en memoria
typedef struct {
    int32_t physical_address;
    int32_t size;
    void* data;
} t_memory_write_request;

/**
 * @brief Crea un objeto de solicitud de escritura en memoria.
 * 
 * @param physical_address Dirección física donde escribir.
 * @param size Tamaño del dato a escribir.
 * @param data Puntero al dato a escribir.
 * @return Puntero a la estructura t_memory_write_request creada.
 */
t_memory_write_request* create_memory_write_request(int32_t physical_address, int32_t size, void* data);

/**
 * @brief Destruye un objeto de solicitud de escritura en memoria.
 * 
 * @param request Puntero a la estructura a destruir.
 */
void destroy_memory_write_request(t_memory_write_request* request);

/**
 * @brief Envía una solicitud de escritura en memoria a través de un socket.
 * 
 * @param socket El socket para enviar la solicitud.
 * @param request La solicitud a enviar.
 * @return 0 si tuvo éxito, -1 si hubo un error.
 */
int send_memory_write_request(int socket, t_memory_write_request* request);

/**
 * @brief Recibe una solicitud de escritura en memoria desde un paquete.
 * 
 * @param package El paquete que contiene la solicitud.
 * @return Puntero a la estructura t_memory_write_request creada.
 */
t_memory_write_request* read_memory_write_request(t_package* package);

#endif