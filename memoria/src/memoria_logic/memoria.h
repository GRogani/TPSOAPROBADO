#ifndef MEMORIA_LOGIC_H
#define MEMORIA_LOGIC_H

#include <commons/collections/list.h>
#include "../utils.h"
#include "semaphores.h"

typedef struct {
    int pid;
    size_t process_size;
    t_list* instructions;
} proc_memory;

typedef struct {
    t_list* processes;

} glb_memory;

extern glb_memory global_memory;

proc_memory* find_process_by_pid(int pid);

void init_process(int socket, t_package* package);

int create_process(uint32_t pid, uint32_t size, char* path_to_script);

void get_instruction(int socket, t_package *package);

void get_free_space(int socket);

void delete_process(int socket, t_package *package);

#endif