#ifndef IO_SERVER_HANDLER_H
#define IO_SERVER_HANDLER_H

#define PUERTO_IO_ESCUCHA "30002"

#include <utils/socket/server.h>
#include "handle-io-client.h"
#include "globals.h"

void create_io_server();

#endif