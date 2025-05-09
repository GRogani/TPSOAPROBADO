#ifndef UTILS_DTOS_H
#define UTILS_DTOS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "../../io/src/data_request.h" // TODO: ver cómo manejar estos includes que ensucian

t_package* create_handshake(char* yourName);

char* read_handshake(t_package* package);

/**
 * @brief Manda un mensaje de handshake
 * @note srteam: `[HANDSHAKE]-[size of stream]-[strlen+1 + "string"]`
*/
int send_handshake(int socket, char* yourName);


t_request_IO* read_IO_operation_request(t_package* package);
t_package* create_IO_operation_request(uint32_t pid, uint32_t time);
int send_IO_operation_request(int socket, uint32_t pid, uint32_t time);

char* read_IO_operation_completed(t_package* package);
int send_IO_operation_completed(int kernel_socket, char* yourName);
t_package* create_IO_operation_completed(char* yourName);

#endif