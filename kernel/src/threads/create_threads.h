#ifndef THREADS_CREATE_SERVERS_H
#define THREADS_CREATE_SERVERS_H


#include <pthread.h>
#include "../utils.h"
#include "handlers/cpu_server.h"
#include "handlers/io_server.h"

int create_servers_threads(pthread_t* io_thread, pthread_t* cpu_thread);

#endif