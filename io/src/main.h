#ifndef IO_MAIN_H
#define IO_MAIN_H

#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../utils.h"
#include "io_threads.h"

void waiting_requests(int kernel_socket, char* id_IO);
void* processing_operation(void* args);

#endif