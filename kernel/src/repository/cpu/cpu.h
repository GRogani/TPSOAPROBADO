#ifndef KERNEL_CPU_REPOSITORY_H
#define KERNEL_CPU_REPOSITORY_H

#include "collections/collections.h"
#include "commons/string.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

void initialize_repository_cpu(sem_t *);

void destroy_repository_cpu(sem_t *);

void lock_cpu(sem_t *);

void unlock_cpu(sem_t *);

#endif