#ifndef IO_CLIENT_HANDLER_H
#define IO_CLIENT_HANDLER_H

#include <pthread.h>
#include "../utils.h"
#include "./io/handshake.h"

void* handle_io_client(void*);
void process_handshake(t_package*, int);

#endif