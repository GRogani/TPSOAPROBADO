#ifndef UTILS_NEW_IO_H
#define UTILS_NEW_IO_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

t_package* create_new_io_package( char* device_name );

int send_new_io_package( int kernel_socket, char* device_name );

char* read_new_io_package( t_package* package );

#endif