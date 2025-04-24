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
    cod_instruction instruction;
    t_list* operands;
} instruction_t;

void request_instruction(int socket, int PC);
int* receive_PID_PC(int socket);
instruction_t* receive_instruction(int socket);

#endif