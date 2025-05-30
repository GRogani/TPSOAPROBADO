#ifndef UTILS_CPU_SYSCALL_H
#define UTILS_CPU_SYSCALL_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef enum {
    SYSCALL_INIT_PROC,
    SYSCALL_IO,
    SYSCALL_DUMP_PROCESS,
    SYSCALL_EXIT
} t_syscall_type;

typedef struct t_cpu_syscall {
    t_syscall_type syscall_type;
    uint32_t pid;
    uint32_t pc;
    union {
        struct {
            uint32_t memory_space;
            char* pseudocode_file;
        } init_proc;
        struct {
            char* device_name;
            uint32_t sleep_time;
        } io;
    } params;
} t_cpu_syscall;

t_cpu_syscall* read_cpu_syscall(t_package* package);

t_package* create_cpu_syscall_response(uint32_t status);

int send_cpu_syscall_response(int socket, uint32_t status);

void destroy_cpu_syscall(t_cpu_syscall* syscall);

#endif
