#ifndef UTILS_CPU_SYSCALL_RESPONSE_H
#define UTILS_CPU_SYSCALL_RESPONSE_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef enum {
    SYSCALL_RESPONSE_INIT_PROC,
    SYSCALL_RESPONSE_IO,
    SYSCALL_RESPONSE_DUMP_PROCESS,
    SYSCALL_RESPONSE_EXIT
} t_syscall_request_type;

typedef struct t_cpu_syscall_request {
    t_syscall_request_type syscall_type;
    uint32_t pid;
    uint32_t pc;
} t_cpu_syscall_request;

t_cpu_syscall_request* read_cpu_syscall_request(t_package* package);

t_package* create_cpu_syscall_request(t_syscall_request_type syscall_type, uint32_t pid, uint32_t pc);

int send_cpu_syscall_request(int socket, t_syscall_request_type syscall_type, uint32_t pid, uint32_t pc);

void destroy_cpu_syscall_request(t_cpu_syscall_request* response);

#endif
