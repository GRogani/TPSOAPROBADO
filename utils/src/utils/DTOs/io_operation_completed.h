#ifndef UTILS_IO_OPERATION_COMPLETED_H
#define UTILS_IO_OPERATION_COMPLETED_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

char* read_io_operation_completed(t_package*);
t_package* create_io_operation_completed(char*);
int send_io_operation_completed(int, char*);

#endif