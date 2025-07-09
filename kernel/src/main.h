#ifndef KERNEL_MAIN_H
#define KERNEL_MAIN_H

#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>

#include "../utils.h"
#include "globals.h"
#include "kernel_logic/shutdown/shutdown.h"
#include "handlers/cpu/server/cpu_server.h"
#include "handlers/io/server/io_server.h"
#include "collections/collections.h"
#include "kernel_logic/threads/create_threads.h"
#include "semaphore/semaphore.h"
#include "kernel_logic/process_initialization.h"

int main(int, char*[]);
void process_enter();

#endif