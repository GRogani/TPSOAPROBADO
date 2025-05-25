#ifndef MEMORIA_H
#define MEMORIA_H

#include <commons/config.h>
#include <commons/log.h>
#include "../../utils/src/utils/config/t_configs.h"
#include "../../utils/src/utils/config/config.h"
#include "../../utils/src/utils/logger/logger.h"
#include "../../utils/src/utils/socket/server.h"
#include "../../utils/src/utils/socket/client.h"
#include "../../utils/src/utils/shutdown.h"

#define MEMORIA_DISPONIBLE 1024

t_memoria_config memoria_config;
t_log* logger;

typedef struct {
    int pid;
    size_t process_size;
    t_list* instructions;          // List of char* (each line from script)
} ProcessMemory;

typedef struct {
    t_list* processes;

} GlobalMemory;


ProcessMemory* find_process_by_pid(int pid);
void create_process(int socket, t_buffer* buffer)
int create_process_in_memory(uint32_t pid, uint32_t size, char* path_to_script);
void get_instruction(int socket, t_buffer* request_buffer);
void get_instructions(int socket, t_buffer* request_buffer);
void get_free_space(int socket);
#endif