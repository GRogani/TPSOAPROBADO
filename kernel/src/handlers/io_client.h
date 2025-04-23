#ifndef IO_CLIENT_HANDLER_H
#define IO_CLIENT_HANDLER_H

#include <pthread.h>
#include "../utils.h"

void* handle_io_client(void* socket);

#endif