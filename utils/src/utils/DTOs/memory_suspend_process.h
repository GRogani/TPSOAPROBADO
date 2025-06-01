#ifndef UTILS_MEMORY_CREATE_PROCESS_H
#define UTILS_MEMORY_CREATE_PROCESS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

/*
typedef struct t_memory_suspend_process {
} t_memory_suspend_process;
*/

// REQUEST

// t_memory_suspend_process* read_memory_suspend_process(t_package *package);
t_package *create_memory_create_process(uint32_t pid);
int send_memory_suspend_process(int socket, uint32_t pid);
// void destroy_memory_suspend_process(t_memory_suspend_process* request);

// RESPONSE


#endif