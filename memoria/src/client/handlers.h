#ifndef HANDLERS_H
#define HANDLERS_H

#include <pthread.h>
#include "../utils.h"
#include "kernel_space/process_manager.h"
#include "swap_space/swap_manager.h"
#include "./init-process/init-process.h"
#include "./fetch-instruction/fetch-instruction.h"
#include "./kill-process/kill-process.h"
#include "./get-page-entry/get-page-entry.h"
#include "./read-memory/read-memory.h"
#include "./write-memory/write-memory.h"
#include "./swap/swap.h"
#include "./unsuspend-process/unsuspend-process.h"
#include "./dump-memory/dump-memory.h"
#include "../delay_utils.h"

int create_server_thread(pthread_t* listener_thread);

void* client_listener(void* arg);

void* client_handler(void* client_fd_ptr);


#endif