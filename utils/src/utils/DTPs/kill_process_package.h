#ifndef DTP_KILL_PROCESS_H
#define DTP_KILL_PROCESS_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

t_package *create_kill_process_package(int32_t pid);

int send_kill_process_package(int socket, int32_t pid);

int32_t read_kill_process_package(t_package *package);

#endif