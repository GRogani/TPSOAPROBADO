#ifndef SHUTDOWN_KERNEL_MAIN_H
#define SHUTDOWN_KERNEL_MAIN_H

#include "globals.h"

void shutdown_hook();
void io_connections_destroyer(void* ptr);
void io_queue_destroyer(void* ptr);
void cpu_connections_destroyer(void* ptr);

#endif