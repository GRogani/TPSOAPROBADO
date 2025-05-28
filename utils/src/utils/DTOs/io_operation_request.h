#ifndef UTILS_IO_OPERATION_REQUEST_H
#define UTILS_IO_OPERATION_REQUEST_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct t_request_io {
    int kernel_socket;
    char* device_name;
    uint32_t pid;
    uint32_t sleep_time;
} t_request_io;

t_request_io* read_io_operation_request(t_package* package);
t_package* create_io_operation_request(uint32_t pid, uint32_t time);
int send_io_operation_request(int socket, uint32_t pid, uint32_t time);

#endif