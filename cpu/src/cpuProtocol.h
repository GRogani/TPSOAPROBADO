#ifndef CPU_PROTOCOL_H
#define CPU_PROTOCOL_H

#include <utils/serialization/package.h>
#include <utils/serialization/buffer.h>
#include <utils/logger/logger.h>
#include <commons/collections/list.h>

typedef enum {
NOOP,
WRITE,
READ,
GOTO,
IO,
INIT_PROC,
DUMP_PROCESS,
EXIT
} cod_instruction;

typedef struct{
    cod_instruction cod_instruction;
    t_list* operands;
} instruction_t;

void request_instruction(int socket_memory, int PC);
int receive_PID(int socket_dispatch_kernel);
int receive_PC(int socket_dispatch_kernel);
instruction_t* receive_instruction(int socket_memory);
void write_memory_request(int socket_memory, uint32_t direccion_fisica, char* valor_write);
void read_memory_request(int socket_memory, uint32_t direccion_fisica, uint32_t size);
char* read_memory_response(int socket_memory);
#endif