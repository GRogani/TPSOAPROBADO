#ifndef CPU_SERVER_HANDLER_H
#define CPU_SERVER_HANDLER_H

#include <utils/socket/server.h>
#include <sys/socket.h>
#include <unistd.h> // para socket close()
#include "collections/collections.h"
#include "semaphore/semaphore.h"
#include "handlers/cpu/client/handle_dispatch_client.h"
#include "repository/cpu/cpu_connections.h"

int connect_to_cpus(int);

int create_cpu_servers();

#endif