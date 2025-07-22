#ifndef KERNEL_READY_LIST_REPOSITORY_H
#define KERNEL_READY_LIST_REPOSITORY_H

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

void initialize_repository_ready();
void destroy_repository_ready();

void lock_ready_list();
void unlock_ready_list();

bool find_pcb_in_ready(int32_t pid);
void add_pcb_to_ready(t_pcb* pcb);
t_pcb* remove_pcb_from_ready(int32_t pid);
t_pcb* get_next_pcb_from_ready(); // Para algoritmos de planificación

#endif
