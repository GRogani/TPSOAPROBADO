#ifndef CPU_PROTOCOL_H
#define CPU_PROTOCOL_H

#include <semaphore.h>
#include <commons/collections/list.h>
#include "../utils.h"
#include "lists_and_mutex.h"

typedef struct {
    int socket_interrupt;
    _Atomic int32_t* pid;
    _Atomic int32_t* pc;
    int socket_memory;
} interrupt_args_t;

//void request_instruction (int socket,int32_t PID, int32_t PC);

t_package* recv_dispatch (int socket_dispatch_kernel, _Atomic int32_t* PID, _Atomic int32_t* PC);

t_package* receive_instruction (int socket);

void create_connections (t_cpu_config config_cpu, int* fd_memory, int* fd_kernel_dispatch, int* fd_kernel_interrupt);


#endif