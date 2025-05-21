#ifndef IO_CLIENT_HANDSHAKE_H
#define IO_CLIENT_HANDSHAKE_H

#include <collections/collections.h>
#include "process_pending_io.h"

typedef struct t_handshake_thread_args {
    char* device_name;
    int client_socket;
} t_handshake_thread_args;

void handsake(void*);
void call_process_pending_io(int, char *);

#endif