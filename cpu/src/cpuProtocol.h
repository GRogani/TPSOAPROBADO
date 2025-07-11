#ifndef CPU_PROTOCOL_H
#define CPU_PROTOCOL_H

#include <semaphore.h>
#include <commons/collections/list.h>
#include "../utils.h"
#include "lists_and_mutex.h"

typedef struct {
    int socket_interrupt;
    uint32_t* pid;
    uint32_t* pc;
    int socket_memory;
} interrupt_args_t;

void request_instruction (int socket,uint32_t PID, uint32_t PC);

t_package* recv_dispatch (int socket_dispatch_kernel, uint32_t* PID, uint32_t* PC);

t_package* receive_instruction (int socket);

void create_connections (t_cpu_config config_cpu, int* fd_memory, int* fd_kernel_dispatch, int* fd_kernel_interrupt);


#endif