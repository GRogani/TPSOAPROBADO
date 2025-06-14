#ifndef CPU_LIST_AND_MUTEX_H
#define CPU_LIST_AND_MUTEX_H

#include <semaphore.h>
#include <commons/collections/list.h>
#include "../utils.h"

void init_list_and_mutex();

void lock_interrupt_list();

void unlock_interrupt_list();

void add_interrupt(t_package* interrupt);

int interrupt_count();

t_package* get_last_interrupt(int i);

void wait_interrupt();

void signal_interrupt();

void lock_cpu_mutex();

void unlock_cpu_mutex();

void destroy_list_and_mutex();

bool should_interrupt_thread_exit();

void signal_interrupt_thread_exit();

#endif 