#ifndef KERNEL_SUSP_BLOCKED_LIST_REPOSITORY_H
#define KERNEL_SUSP_BLOCKED_LIST_REPOSITORY_H

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

bool initialize_repository_susp_blocked();
bool destroy_repository_susp_blocked();

void lock_susp_blocked_list();
void unlock_susp_blocked_list();

bool find_pcb_in_susp_blocked(uint32_t pid);
void add_pcb_to_susp_blocked(t_pcb* pcb);
t_pcb* remove_pcb_from_susp_blocked(uint32_t pid);

#endif
