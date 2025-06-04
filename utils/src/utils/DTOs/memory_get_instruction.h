#ifndef UTILS_MEMORY_GET_INSTRUCTION_H
#define UTILS_MEMORY_GET_INSTRUCTION_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct t_get_instruction_request {
    uint32_t pid;
    uint32_t pc;
} t_get_instruction;

// Request packages
t_get_instruction read_memory_get_instruction_package(t_package* package);

t_package* create_memory_get_instruction_package(uint32_t pid, uint32_t pc);

void send_memory_get_instruction_package(int socket, uint32_t pid, uint32_t pc);

void destroy_memory_get_instruction_package(t_get_instruction request);

// Response packages
char *read_memory_instruction_package(t_package *package);

t_package *create_memory_instruction_package(char *instruction);

int send_memory_instruction_package(int socket, char* instruction);

#endif
