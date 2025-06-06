#ifndef DTP_IO_COMPLETION_H
#define DTP_IO_COMPLETION_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

t_package* create_io_completion_package (char*);

int send_io_completion_package (int, char*);

char* read_io_completion__package (t_package*);

#endif