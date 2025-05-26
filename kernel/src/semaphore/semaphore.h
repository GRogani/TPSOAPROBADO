#ifndef KERNEL_GLOBAL_SEMAPHORES_H
#define KERNEL_GLOBAL_SEMAPHORES_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "../utils.h"

void initialize_global_semaphores();

void initialize_cpu_connected_sem();
void destroy_cpu_connected_sem();

void wait_cpu_connected();
void signal_cpu_connected();

#endif