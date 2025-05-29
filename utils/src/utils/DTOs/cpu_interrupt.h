#ifndef UTILS_CPU_INTERRUPT_H
#define UTILS_CPU_INTERRUPT_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct t_cpu_interrupt {
    uint32_t pid;
} t_cpu_interrupt;

t_cpu_interrupt* read_cpu_interrupt(t_package* package);

t_package* create_cpu_interrupt_request(uint32_t pid);

int send_cpu_interrupt_request(int socket, uint32_t pid);

void destroy_cpu_interrupt(t_cpu_interrupt* interrupt);

#endif
