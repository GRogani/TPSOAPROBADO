#ifndef PROCESS_PENDING_IO_H
#define PROCESS_PENDING_IO_H

#include <collections/collections.h>
#include "../utils.h"
#include "repository/process/blocked_list.h"
#include "repository/process/susp_blocked_list.h"

typedef struct t_pending_io_args
{
    char* device_name;
    int client_socket;
} t_pending_io_args;

bool process_pending_io(t_pending_io_args);

#endif