#ifndef DTP_DUMP_MEMORY_H
#define DTP_DUMP_MEMORY_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"


t_package *create_dump_memory_package(int32_t pid);

int send_dump_memory_package(int socket, int32_t pid);

int32_t read_dump_memory_package(t_package *package);

#endif