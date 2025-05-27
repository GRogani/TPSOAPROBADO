#ifndef KERNEL_CPU_CONNECTIONS_REPOSITORY_H
#define KERNEL_CPU_CONNECTIONS_REPOSITORY_H

#include "collections/collections.h"
#include "commons/string.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

bool initialize_repository_cpu_connections();
bool destroy_repository_cpu_connections();

void lock_cpu_connections();
void unlock_cpu_connections();

void create_cpu_connection(int socket_dispatch, int socket_interrupt)

#endif