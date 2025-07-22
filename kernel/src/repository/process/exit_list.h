#ifndef KERNEL_EXIT_LIST_REPOSITORY_H
#define KERNEL_EXIT_LIST_REPOSITORY_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include "../utils.h"
#include "repository/pcb/pcb.h"
#include "collections/collections.h"
#include <commons/collections/list.h>

// Forward declaration
typedef struct t_pcb t_pcb;

void initialize_repository_exit();
void destroy_repository_exit();

void lock_exit_list();
void unlock_exit_list();

bool find_pcb_in_exit(int32_t pid);
void add_pcb_to_exit(t_pcb* pcb);
void remove_pcb_from_exit(int32_t pid);

#endif
