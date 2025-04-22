#ifndef THREADS_KERNEL_CONNECTION_H
#define THREADS_KERNEL_CONNECTION_H

#include <unistd.h>
#include "../utils.h"

void* create_kernel_connection(void* connection_socket);

#endif