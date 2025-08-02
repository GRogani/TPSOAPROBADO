#ifndef IO_CLIENT_COMPLETION_H
#define IO_CLIENT_COMPLETION_H

#include <collections/collections.h>
#include <pthread.h>
#include "../utils.h"
#include "repository/io/io_connections.h"
#include "repository/process/blocked_list.h"
#include "repository/process/susp_blocked_list.h"
#include "repository/process/ready_list.h"
#include "repository/process/susp_ready_list.h"
#include "kernel_logic/scheduler/short_scheduler.h"
#include "kernel_logic/scheduler/long_scheduler.h"
#include "process_pending_io.h"

typedef struct t_completion_thread_args {
    char* device_name;
    int client_socket;
    int32_t pid;
} t_completion_thread_args;

typedef struct t_pending_io_thread_args {
    t_pending_io_args pending_args;
} t_pending_io_thread_args;

void* io_completion(void*);
void* process_pending_io_thread(void* args);
void process_schedulers_for_susp_blocked();
void process_scheduler_for_blocked();

#endif