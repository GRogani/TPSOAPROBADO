#ifndef KERNEL_EXEC_LIST_REPOSITORY_H
#define KERNEL_EXEC_LIST_REPOSITORY_H

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

void initialize_repository_exec();
void destroy_repository_exec();

void lock_exec_list();
void unlock_exec_list();

void *find_pcb_in_exec(int32_t pid); 
void add_pcb_to_exec(t_pcb *pcb);
t_pcb* remove_pcb_from_exec(int32_t pid);

#endif
