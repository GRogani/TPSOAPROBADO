#ifndef IO_SERVER_HANDLER_H
#define IO_SERVER_HANDLER_H

//#define PUERTO_IO_ESCUCHA "30002"

#include <utils/socket/server.h>
#include "io_client.h"


void* io_server_handler(void* args);

#endif