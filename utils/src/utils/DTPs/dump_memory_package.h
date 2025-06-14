#ifndef DTP_DUMP_MEMORY_H
#define DTP_DUMP_MEMORY_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"


t_package *create_dump_memory_package(uint32_t pid);

int send_dump_memory_package(int socket, uint32_t pid);

uint32_t read_dump_memory_package(t_package *package);

#endif