#ifndef CPU_DISPATCH_HANDLER_H
#define CPU_DISPATCH_HANDLER_H

#include <pthread.h>
#include "collections/collections.h"
#include "semaphore/semaphore.h"
#include "../utils.h"
#include "handlers/cpu/syscall/exit_syscall.h"
#include "handlers/cpu/syscall/io_syscall.h"
#include "handlers/cpu/syscall/dump_memory_syscall.h"
#include "handlers/cpu/syscall/init_proc_syscall.h"

void *handle_dispatch_client(void *);
void handle_cpu_syscall(t_package* package, int socket);

#endif