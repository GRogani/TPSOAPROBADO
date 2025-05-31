#ifndef UTILS_IO_NEW_DEVICE_H
#define UTILS_IO_NEW_DEVICE_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

char* read_new_device( t_package* package );

t_package* create_new_device( char* device_name );

int send_new_device( int kernel_socket, char* device_name );

#endif