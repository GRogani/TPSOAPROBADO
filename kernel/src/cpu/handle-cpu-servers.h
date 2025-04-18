#ifndef CPU_SERVER_HANDLER_H
#define CPU_SERVER_HANDLER_H

#include <utils/socket/server.h>
#include <sys/socket.h>
#include <unistd.h> // para socket close()
#include "globals.h"

#define PUERTO_CPU_DISPATCH_ESCUCHA "30003"
#define PUERTO_CPU_INTERRUPT_ESCUCHA "30004"

void create_cpu_servers();
int add_cpu_connection(int socket_dispatch, int socket_interrupt);

#endif