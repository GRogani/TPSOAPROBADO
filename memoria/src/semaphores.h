#ifndef MEMORIA_SEMAPHORES_H
#define MEMORIA_SEMAPHORES_H

#include <semaphore.h>
#include <stdint.h>
#include "../utils.h"


// Initialization and Destruction
void initialize_memory_semaphores();
void destroy_memory_semaphores();

// Lock/Unlock functions for the process list
void lock_process_list();
void unlock_process_list();

// Lock/Unlock functions for the frame manager
void lock_frame_manager();
void unlock_frame_manager();

// Lock/Unlock functions for process instructions (if needed)
void lock_process_instructions();
void unlock_process_instructions();

#endif