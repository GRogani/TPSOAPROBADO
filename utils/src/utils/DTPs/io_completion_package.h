#ifndef DTP_IO_COMPLETION_H
#define DTP_IO_COMPLETION_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct io_completion_package_data {
    char* device_name;
    int32_t pid;
} io_completion_package_data;

t_package* create_io_completion_package (char*, int32_t);

int send_io_completion_package (int, char*, int32_t);

io_completion_package_data* read_io_completion_package (t_package*);

#endif