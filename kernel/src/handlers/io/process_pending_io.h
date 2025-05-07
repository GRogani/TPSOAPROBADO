#ifndef PROCESS_PENDING_IO_H
#define PROCESS_PENDING_IO_H

#include <lists/lists.h>
#include <utils/DTOs/dtos.h>

typedef struct t_pending_io_args
{
    char* device_name;
    int client_socket;
} t_pending_io_args;

void process_pending_io(t_pending_io_args);

#endif