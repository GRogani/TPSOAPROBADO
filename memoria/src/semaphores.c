#include "semaphores.h"

// Implementación de funciones de semáforos
void initialize_memory_semaphores() {
    if (sem_init(&sem_global_processes, 0, 1) != 0) {
        LOG_ERROR("sem_init for global_processes failed");
        exit(EXIT_FAILURE);
    }
    
    if (sem_init(&sem_process_instructions, 0, 1) != 0) {
        LOG_ERROR("sem_init for process_instructions failed");
        sem_destroy(&sem_global_processes);
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Memory semaphores initialized");
}

void destroy_memory_semaphores() {
    sem_destroy(&sem_global_processes);
    sem_destroy(&sem_process_instructions);
}

void lock_global_processes() {
    sem_wait(&sem_global_processes);
}

void unlock_global_processes() {
    sem_post(&sem_global_processes);
}

void lock_process_instructions() {
    sem_wait(&sem_process_instructions);
}

void unlock_process_instructions() {
    sem_post(&sem_process_instructions);
}
