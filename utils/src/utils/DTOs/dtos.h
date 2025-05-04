#ifndef UTILS_DTOS_H
#define UTILS_DTOS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"


t_package* create_handshake(char* yourName);

char* read_handshake(t_package* package);

/**
 * @brief Manda un mensaje de handshake
 * @note stream: `[HANDSHAKE]-[size of stream]-[strlen+1 + "string"]`
*/
int send_handshake(int socket, char* yourName);


t_package* create_io_request(uint32_t pid, uint32_t sleep_time);

/**
 * @brief Manda un mensaje de pedido de utilizar la IO
 * @note stream: `[IO_REQUEST]-[size of stream]-[strlen+1 + "string"]`
*/
int send_io_request(int socket, uint32_t pid, uint32_t sleep_time);


#endif