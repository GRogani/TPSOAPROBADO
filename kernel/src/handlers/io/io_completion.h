#ifndef IO_CLIENT_COMPLETION_H
#define IO_CLIENT_COMPLETION_H

#include <collections/collections.h>

typedef struct t_completion_thread_args {
    char* device_name;
    int client_socket;
} t_completion_thread_args;

void* io_completion(void*);

#endif