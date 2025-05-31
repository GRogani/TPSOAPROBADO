#ifndef IO_CLIENT_NEW_DEVICE_H
#define IO_CLIENT_NEW_DEVICE_H

#include <collections/collections.h>
#include "process_pending_io.h"

typedef struct t_new_device_thread_args {
    char* device_name;
    int client_socket;
} t_new_device_thread_args;

void* process_new_device(void*);
void call_process_pending_io(int, char *);

#endif