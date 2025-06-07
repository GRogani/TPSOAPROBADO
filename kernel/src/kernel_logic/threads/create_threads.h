#ifndef THREADS_CREATE_SERVERS_H
#define THREADS_CREATE_SERVERS_H


#include <pthread.h>
#include "../utils.h"
#include "handlers/cpu/server/cpu_server.h"
#include "handlers/io/server/io_server.h"

int create_servers_threads(pthread_t* io_thread);

#endif