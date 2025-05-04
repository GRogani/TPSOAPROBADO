#ifndef CPU_SERVER_HANDLER_H
#define CPU_SERVER_HANDLER_H

#include <utils/socket/server.h>
#include <sys/socket.h>
#include <unistd.h> // para socket close()
#include "lists/lists.h"

void* cpu_server_handler(void*);

#endif