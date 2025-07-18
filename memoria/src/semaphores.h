#ifndef MEMORIA_SEMAPHORES_H
#define MEMORIA_SEMAPHORES_H

#include <semaphore.h>
#include <stdint.h>
#include "../../utils/utils.h"

void initialize_memory_semaphores();
void destroy_memory_semaphores();

void lock_process_list();
void unlock_process_list();

void lock_frame_manager();
void unlock_frame_manager();

void lock_process_instructions();
void unlock_process_instructions();

void lock_swap_file();
void unlock_swap_file();

void lock_page_table();
void unlock_page_table();

void lock_process_metrics();
void unlock_process_metrics();

void lock_process_creation();
void unlock_process_creation();

#endif