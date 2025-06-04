#ifndef UTILS_CPU_SYSCALL_H
#define UTILS_CPU_SYSCALL_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"
#include "enums/Esyscall_type.h"

typedef struct syscall_package_data
{
    SYSCALL_TYPE syscall_type;
    uint32_t pid;
    uint32_t pc;
    union
    {
        struct
        {
            char *pseudocode_file;
            uint32_t memory_space;
        } init_proc;
        struct
        {
            char *device_name;
            uint32_t sleep_time;
        } io;
    } params;
} syscall_package_data;

syscall_package_data *read_syscall_package(t_package *package);

t_package *create_syscall_package(syscall_package_data *syscall);

int send_syscall_package(int socket, syscall_package_data *syscall);

void destroy_syscall_package(syscall_package_data *syscall);


#endif
