#ifndef IO_MAIN_H
#define IO_MAIN_H

#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../utils.h"
#include "io_threads.h"

//volatile bool io_busy = false;
//pthread_mutex_t busy_mutex = PTHREAD_MUTEX_INITIALIZER;

void waiting_requests(int kernel_socket, char* id_IO);
void processing_operation(io_operation_package_data* args);

#endif