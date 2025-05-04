#ifndef PROCESS_PENDING_IO_H
#define PROCESS_PENDING_IO_H

#include <lists/lists.h>
#include <utils/DTOs/dtos.h>

typedef struct t_pending_io_thread_args {
    char* device_name;
    int client_socket;
} t_pending_io_thread_args;


void process_pending_io(void*);

#endif