#include "semaphores.h"

sem_t sem_process_list;
sem_t sem_frame_manager;
sem_t sem_process_instructions;

void initialize_memory_semaphores() {
    if (sem_init(&sem_process_list, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_process_list failed");
    }

    if (sem_init(&sem_frame_manager, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_frame_manager failed");
        sem_destroy(&sem_process_list); 
    }

    if (sem_init(&sem_process_instructions, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_process_instructions failed");
        sem_destroy(&sem_process_list);
        sem_destroy(&sem_frame_manager);
    }

    LOG_INFO("Memory semaphores initialized.");
}

void destroy_memory_semaphores() {
    sem_destroy(&sem_process_list);
    sem_destroy(&sem_frame_manager);
    sem_destroy(&sem_process_instructions);
    LOG_INFO("Memory semaphores destroyed.");
}

void lock_process_list() {
    sem_wait(&sem_process_list);
}

void unlock_process_list() {
    sem_post(&sem_process_list);
}

void lock_frame_manager() {
    sem_wait(&sem_frame_manager);
}

void unlock_frame_manager() {
    sem_post(&sem_frame_manager);
}

void lock_process_instructions() {
    sem_wait(&sem_process_instructions);
}

void unlock_process_instructions() {
    sem_post(&sem_process_instructions);
}