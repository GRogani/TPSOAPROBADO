#ifndef UTILS_CPU_INTERRUPT_H
#define UTILS_CPU_INTERRUPT_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct t_cpu_interrupt {
    uint32_t pid;
    uint32_t pc;
} t_cpu_interrupt;


int read_cpu_interrupt_request(t_package *package);

t_package* create_cpu_interrupt_request(uint32_t pid);

int send_cpu_interrupt_request(int socket, uint32_t pid);

// New functions for interrupt response with PC
t_cpu_interrupt *read_cpu_interrupt_response(t_package *package);

t_package* create_cpu_interrupt_response(uint32_t pid, uint32_t pc);

int send_cpu_interrupt_response(int socket, uint32_t pid, uint32_t pc);

void destroy_cpu_interrupt(t_cpu_interrupt* interrupt);

#endif
