#ifndef CPU_PROTOCOL_H
#define CPU_PROTOCOL_H

#include <commons/collections/list.h>
#include "../utils.h"
#include "utils/DTOs/cpu_dispatch.h"
#include "utils/DTOs/memory_get_instruction.h"

typedef enum {
INST_NOOP,
INST_WRITE,
INST_READ,
INST_GOTO,
INST_IO,
INST_INIT_PROC,
INST_DUMP_PROCESS,
INST_EXIT
} cod_instruction;

typedef struct{
    cod_instruction cod_instruction;
    uint32_t operand_numeric1;
    uint32_t operand_numeric2;
    uint32_t operand_string_size;
    char* operand_string;
} instruction_t;

typedef struct {
    int socket_interrupt;
    uint32_t* pid;
    uint32_t* pc;
} interrupt_args_t;

void request_instruction(int socket,uint32_t PID, uint32_t PC);

t_package* receive_PID_PC_Package(int socket_dispatch_kernel, uint32_t* PID, uint32_t* PC);

t_package* receive_instruction(int socket);

void write_memory_request(int socket, uint32_t direccion_fisica, char* valor_write);

void read_memory_request(int socket, uint32_t direccion_fisica, uint32_t size);

char* read_memory_response(int socket);

void create_connections(t_cpu_config config_cpu, int* fd_memory, int* fd_kernel_dispatch, int* fd_kernel_interrupt);


#endif