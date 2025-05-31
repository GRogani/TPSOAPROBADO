#ifndef UTILS_MEMORY_CREATE_PROCESS_H
#define UTILS_MEMORY_CREATE_PROCESS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct t_memory_create_process {
    uint32_t pid;
    uint32_t size;
    char* pseudocode_path;
} t_memory_create_process;

t_memory_create_process* read_memory_create_process_request(t_package* package);
t_package* create_memory_create_process_request(uint32_t pid, uint32_t size, char* pseudocode_path);
int send_memory_create_process_request(int socket, uint32_t pid, uint32_t size, char* pseudocode_path);
void destroy_memory_create_process(t_memory_create_process* request);

bool read_memory_create_process_response(t_package *package);
t_package *create_memory_create_process_response(uint32_t success);
int send_memory_create_process_response(int socket, uint32_t success);

#endif