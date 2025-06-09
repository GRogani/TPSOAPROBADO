#ifndef DTP_KILL_PROCESS_MEMORY_H
#define DTP_KILL_PROCESS_MEMORY_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

t_package *create_kill_process_package(uint32_t pid);

int send_kill_process_package(int socket, uint32_t pid);

uint32_t read_kill_process_package(t_package *package);

#endif