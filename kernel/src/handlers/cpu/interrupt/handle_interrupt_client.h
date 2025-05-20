#ifndef CPU_INTERRUPT_HANDLER_H
#define CPU_INTERRUPT_HANDLER_H

#include <pthread.h>
#include <utils/serialization/package.h>
#include "collections/collections.h"
#include "semaphore/semaphore.h"

void *handle_interrupt_client(void *);

#endif