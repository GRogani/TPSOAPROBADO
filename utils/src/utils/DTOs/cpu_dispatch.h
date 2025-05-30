#ifndef UTILS_CPU_DISPATCH_H
#define UTILS_CPU_DISPATCH_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct t_cpu_dispatch {
    uint32_t pid;
    uint32_t pc;
} t_cpu_dispatch;

t_cpu_dispatch* read_cpu_dispatch(t_package* package);

t_package* create_cpu_dispatch_request(uint32_t pid, uint32_t pc);

int send_cpu_dispatch_request(int socket, uint32_t pid, uint32_t pc);

void destroy_cpu_dispatch(t_cpu_dispatch* dispatch);

#endif
