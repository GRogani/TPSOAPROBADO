#ifndef KERNEL_CPU_CONNECTIONS_REPOSITORY_H
#define KERNEL_CPU_CONNECTIONS_REPOSITORY_H

#include "collections/collections.h"
#include "commons/string.h"
#include "cpu.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

void initialize_repository_cpu_connections();
void destroy_repository_cpu_connections();

void lock_cpu_connections();

void* get_cpu_connection_by_id(char *id);
void* get_cpu_connection_by_pid(int32_t pid);
char *create_cpu_connection(int socket_interrupt, int socket_dispatch);
void remove_cpu_connection(char *id);

void unlock_cpu_connections();

t_list *get_all_cpu_connections();
void* get_first_available_cpu();

#endif