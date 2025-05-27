#ifndef UTILS_DTOS_H
#define UTILS_DTOS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

// IO

t_package* create_io_handshake(char* yourName);
char* read_io_handshake(t_package* package);
/**
 * @brief Manda un mensaje de handshake al modulo IO
 * @note stream: `[HANDSHAKE]-[size of stream]-[strlen+1 + "string"]`
*/
int send_io_handshake(int socket, char* yourName);

t_package* create_io_request(uint32_t pid, uint32_t sleep_time);
/**
 * @brief Manda un mensaje de pedido de utilizar la IO
 * @note stream: `[IO_REQUEST]-[size of stream]-[pid size + "int"]-[sleep_time size + "int"]`
*/
int send_io_request(int socket, uint32_t pid, uint32_t sleep_time);

int read_io_completion(t_package*);
t_package *create_io_completion(int);
int send_io_completion(int, int);


void* read_IO_operation_request(t_package* package);
t_package* create_IO_operation_request(uint32_t pid, uint32_t time);
int send_IO_operation_request(int socket, uint32_t pid, uint32_t time);

char* read_IO_operation_completed(t_package* package);
int send_IO_operation_completed(int kernel_socket, char* yourName);
t_package* create_IO_operation_completed(char* yourName);

#endif