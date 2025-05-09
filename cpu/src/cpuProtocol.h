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
    uint32_t operand_numeric1;
    uint32_t operand_numeric2;
    uint32_t operand_string_size;
    char* operand_string;
} instruction_t;

void request_instruction(int socket_memory, int PC);
t_package* receive_PID_PC_Package(int socket_dispatch_kernel);
t_package* receive_instruction(int socket_memory);
void write_memory_request(int socket_memory, uint32_t direccion_fisica, char* valor_write);
void read_memory_request(int socket_memory, uint32_t direccion_fisica, uint32_t size);
char* read_memory_response(int socket_memory);
#endif