#ifndef IO_CLIENT_HANDSHAKE_H
#define IO_CLIENT_HANDSHAKE_H

#include <lists/lists.h>
#include "process_pending_io.h"

typedef struct t_handshake_thread_args {
    char* device_name;
    int client_socket;
} t_handshake_thread_args;

void handsake(void*);
void thread_for_process_next_io(int, char*);

#endif