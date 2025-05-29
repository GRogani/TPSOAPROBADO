#ifndef KERNEL_SUSP_READY_LIST_REPOSITORY_H
#define KERNEL_SUSP_READY_LIST_REPOSITORY_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include "../utils.h"
#include "../pcb.h"
#include "collections/collections.h"

// Forward declaration
typedef struct t_pcb t_pcb;

bool initialize_repository_susp_ready();
bool destroy_repository_susp_ready();

void lock_susp_ready_list();
void unlock_susp_ready_list();

bool find_pcb_in_susp_ready(uint32_t pid);
void add_pcb_to_susp_ready(t_pcb* pcb);
t_pcb* remove_pcb_from_susp_ready(uint32_t pid);
t_pcb* get_next_pcb_from_susp_ready(); // Para algoritmos de planificación

#endif
