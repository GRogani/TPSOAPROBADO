#ifndef DTP_EXECUTION_CONTEXT_H
#define DTP_EXECUTION_CONTEXT_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct cpu_context_package_data {
    uint32_t pid;
    uint32_t pc;
    int32_t interrupted_same_pid;
} cpu_context_package_data;

t_package *create_cpu_context_package (uint32_t pid, uint32_t pc, int32_t interrupted_same_pid);

int send_cpu_context_package (int socket, uint32_t pid, uint32_t pc, int32_t interrupted_same_pid);

cpu_context_package_data read_cpu_context_package (t_package *package);

#endif