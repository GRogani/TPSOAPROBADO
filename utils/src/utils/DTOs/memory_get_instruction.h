#ifndef UTILS_MEMORY_GET_INSTRUCTION_H
#define UTILS_MEMORY_GET_INSTRUCTION_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct t_memory_get_instruction_request {
    uint32_t pid;
    uint32_t pc;
} t_memory_get_instruction_request;

typedef struct t_memory_get_instruction_response {
    char* instruction;
} t_memory_get_instruction_response;

// Request functions
t_memory_get_instruction_request* read_memory_get_instruction_request(t_package* package);

t_package* create_memory_get_instruction_request(uint32_t pid, uint32_t pc);

int send_memory_get_instruction_request(int socket, uint32_t pid, uint32_t pc);

void destroy_memory_get_instruction_request(t_memory_get_instruction_request* request);

// Response functions
t_memory_get_instruction_response* read_memory_get_instruction_response(t_package* package);

t_package* create_memory_get_instruction_response(const char* instruction);

int send_memory_get_instruction_response(int socket, const char* instruction);

void destroy_memory_get_instruction_response(t_memory_get_instruction_response* response);

#endif
