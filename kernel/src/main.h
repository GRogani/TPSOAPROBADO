#ifndef KERNEL_MAIN_H
#define KERNEL_MAIN_H

#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>

#include "utils/config/t_configs.h"
#include "utils/logger/logger.h"
#include "shutdown/shutdown.h"
#include "handlers/cpu_server.h"
#include "handlers/io_server.h"
#include "collections/collections.h"
#include "threads/create_threads.h"
#include "semaphore/semaphore.h"

int main(int, char*[]);
void process_enter();

#endif