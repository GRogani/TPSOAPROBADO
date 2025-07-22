#ifndef DTP_IO_OPERATION_H
#define DTP_IO_OPERATION_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct io_operation_package_data {
    int kernel_socket;
    char* device_name;
    int32_t pid;
    int32_t sleep_time;
} io_operation_package_data;

t_package* create_io_operation_package (int32_t pid, int32_t time);

int send_io_operation_package (int socket, int32_t pid, int32_t time);

io_operation_package_data* read_io_operation_package (t_package* package);

#endif