#ifndef MEMORIA_H
#define MEMORIA_H

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
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

extern glb_memory global_memory;

// Semáforos para proteger acceso concurrente
extern sem_t sem_global_processes;      // Protege global_memory.processes
extern sem_t sem_process_instructions;  // Protege proc_memory->instructions

// Funciones de inicialización y destrucción de semáforos
void initialize_memory_semaphores();
void destroy_memory_semaphores();

// Funciones de lock/unlock
void lock_global_processes();
void unlock_global_processes();
void lock_process_instructions();
void unlock_process_instructions();

proc_memory* find_process_by_pid(int pid);
void create_process(int socket, t_package* package);
int create_process_in_memory(uint32_t pid, uint32_t size, char* path_to_script);
void get_instruction(int socket, t_package *package);
void get_instructions(int socket, t_buffer *request_buffer);
void get_free_space(int socket);
void kill_process(int socket, t_package* package);
int remove_process_from_memory(uint32_t pid);
void destroy_proc_memory(proc_memory* proc);
#endif