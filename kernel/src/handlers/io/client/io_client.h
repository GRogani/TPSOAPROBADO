#ifndef IO_CLIENT_HANDLER_H
#define IO_CLIENT_HANDLER_H

#include <pthread.h>
#include "../utils.h"
#include "handlers/io/process_new_device.h"
#include "handlers/io/io_completion.h"
#include "handlers/io/io_disconnected.h"

void* handle_io_client(void*);
void handle_new_device(t_package*, int);
void process_io_completion(t_package *package, int socket);

#endif