#ifndef CPU_PROTOCOL_H
#define CPU_PROTOCOL_H

#include <semaphore.h>
#include <commons/collections/list.h>
#include "../utils.h"
#include "utils/DTOs/cpu_dispatch.h"
#include "utils/DTOs/memory_get_instruction.h"
#include "utils/DTOs/memory_write_request.h"
#include "utils/DTOs/memory_read_request.h"
#include "utils/DTOs/memory_read_response.h"

typedef struct {
    int socket_interrupt;
    uint32_t* pid;
    uint32_t* pc;
} interrupt_args_t;

void request_instruction(int socket,uint32_t PID, uint32_t PC);

t_package* receive_PID_PC_Package(int socket_dispatch_kernel, uint32_t* PID, uint32_t* PC);

t_package* receive_instruction(int socket);

void create_connections(t_cpu_config config_cpu, int* fd_memory, int* fd_kernel_dispatch, int* fd_kernel_interrupt);


#endif