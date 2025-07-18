#include "semaphores.h"

sem_t sem_process_list;
sem_t sem_frame_manager;
sem_t sem_process_instructions;
sem_t sem_swap_file;
sem_t sem_page_table;
sem_t sem_process_metrics;
sem_t sem_process_creation;

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
    if (sem_init(&sem_swap_file, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_swap_file failed");
        sem_destroy(&sem_process_list);
        sem_destroy(&sem_frame_manager);
        sem_destroy(&sem_process_instructions);
    }
    if (sem_init(&sem_page_table, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_page_table failed");
        sem_destroy(&sem_process_list);
        sem_destroy(&sem_frame_manager);
        sem_destroy(&sem_process_instructions);
        sem_destroy(&sem_swap_file);
    }
    if (sem_init(&sem_process_metrics, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_process_metrics failed");
        sem_destroy(&sem_process_list);
        sem_destroy(&sem_frame_manager);
        sem_destroy(&sem_process_instructions);
        sem_destroy(&sem_swap_file);
        sem_destroy(&sem_page_table);
    }
    
    if (sem_init(&sem_process_creation, 0, 1) != 0) {
        LOG_ERROR("sem_init for sem_process_creation failed");
        sem_destroy(&sem_process_list);
        sem_destroy(&sem_frame_manager);
        sem_destroy(&sem_process_instructions);
        sem_destroy(&sem_swap_file);
        sem_destroy(&sem_page_table);
        sem_destroy(&sem_process_metrics);
    }
    LOG_INFO("Memory semaphores initialized.");
}

void destroy_memory_semaphores() {
    sem_destroy(&sem_process_list);
    sem_destroy(&sem_frame_manager);
    sem_destroy(&sem_process_instructions);
    sem_destroy(&sem_swap_file);
    sem_destroy(&sem_page_table);
    sem_destroy(&sem_process_metrics);
    sem_destroy(&sem_process_creation);
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

void lock_swap_file() {
    sem_wait(&sem_swap_file);
}

void unlock_swap_file() {
    sem_post(&sem_swap_file);
}

void lock_page_table() {
    sem_wait(&sem_page_table);
}

void unlock_page_table() {
    sem_post(&sem_page_table);
}

void lock_process_metrics() {
    sem_wait(&sem_process_metrics);
}

void unlock_process_metrics() {
    sem_post(&sem_process_metrics);
}

void lock_process_creation() {
    sem_wait(&sem_process_creation);
}

void unlock_process_creation() {
    sem_post(&sem_process_creation);
}