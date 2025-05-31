#ifndef MEMORIA_H
#define MEMORIA_H

#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../utils.h"

#define MEMORIA_DISPONIBLE 1024

extern t_memoria_config memoria_config;

typedef struct {
    int pid;
    size_t process_size;
    t_list* instructions;          // List of char* (each line from script)
} proc_memory;

typedef struct {
    t_list* processes;

} glb_memory;


proc_memory* find_process_by_pid(int pid);
void create_process(int socket, t_package* package);
int create_process_in_memory(uint32_t pid, uint32_t size, char* path_to_script);
void get_instruction(int socket, t_package *package);
void get_instructions(int socket, t_buffer *request_buffer);
void get_free_space(int socket);
#endif