#ifndef CPU_DISPATCH_HANDLER_H
#define CPU_DISPATCH_HANDLER_H

#include <pthread.h>
#include <utils/serialization/package.h>
#include "collections/collections.h"
#include "semaphore/semaphore.h"

void *handle_dispatch_client(void *);

#endif