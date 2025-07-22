#ifndef KERNEL_NEW_LIST_REPOSITORY_H
#define KERNEL_NEW_LIST_REPOSITORY_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include "../utils.h"
#include "repository/pcb/pcb.h"
#include "collections/collections.h"

// Forward declaration
typedef struct t_pcb t_pcb;

// Declaration for global list getter
t_list *get_new_list();

void initialize_repository_new();
void destroy_repository_new();

void lock_new_list();
void unlock_new_list();

bool find_pcb_in_new(int32_t pid);
void add_pcb_to_new(t_pcb* pcb);
t_pcb* remove_pcb_from_new(int32_t pid);
t_pcb* get_next_pcb_from_new(); // Para algoritmos de planificación

#endif