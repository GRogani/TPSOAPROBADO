#ifndef UTILS_IO_NEW_DEVICE_H
#define UTILS_IO_NEW_DEVICE_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

char* read_new_device(t_package*);
t_package* create_new_device(char*);
int send_new_device(int, char*);

#endif