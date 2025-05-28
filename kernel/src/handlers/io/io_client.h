#ifndef IO_CLIENT_HANDLER_H
#define IO_CLIENT_HANDLER_H

#include <pthread.h>
#include "../../utils/utils.h"
#include "./process_new_device.h"
#include "./io_completion.h"

void* handle_io_client(void*);
void handle_new_device(t_package*, int);

#endif