#ifndef IO_DISCONNECTED_H
#define IO_DISCONNECTED_H

#include <collections/collections.h>
#include "../cpu/syscall/exit_syscall.h"

void io_disconnected(int socket_id);
void handle_found_process(t_io_connection *io_connection);

#endif