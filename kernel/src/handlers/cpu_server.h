#ifndef CPU_SERVER_HANDLER_H
#define CPU_SERVER_HANDLER_H

#include <utils/socket/server.h>
#include <sys/socket.h>
#include <unistd.h> // para socket close()
#include "collections/collections.h"
#include "semaphore/semaphore.h"

void* cpu_server_handler(void*);

#endif