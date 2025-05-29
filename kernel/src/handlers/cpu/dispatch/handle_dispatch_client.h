#ifndef CPU_DISPATCH_HANDLER_H
#define CPU_DISPATCH_HANDLER_H

#include <pthread.h>
#include <utils/serialization/package.h>
#include "collections/collections.h"
#include "semaphore/semaphore.h"
#include "utils/DTOs/cpu_syscall.h"
#include "../init_proc_syscall.h"

void *handle_dispatch_client(void *);
void handle_cpu_syscall(t_package* package, int socket);

#endif