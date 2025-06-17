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

#endif