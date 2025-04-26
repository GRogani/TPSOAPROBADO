#ifndef UTILS_DTOS_H
#define UTILS_DTOS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

/**
 * @struct t_IO
 * @brief Estructura de una I/O asociada al evento del proceso
 */
typedef struct t_IO {
    char* id;       // Para identificar a un I/O en especifico
    int32_t pid;    // Proceso que espera un evento de I/O
    int32_t time;   // Tiempo que tarda la operación de I/O
} t_IO;


char* read_handshake(t_package* package);
t_package* create_handshake(char* yourName);
int send_handshake(int socket, char* yourName);


t_IO* read_IO_operation_request(t_package* package);
t_package* create_IO_operation_request(int32_t pid, int32_t time);
int send_IO_operation_request(int socket, int32_t pid, int32_t time);

char* read_IO_operation_completed(t_package* package);
int send_IO_operation_completed(int kernel_socket, char* yourName);
t_package* create_IO_operation_completed(char* yourName);

#endif