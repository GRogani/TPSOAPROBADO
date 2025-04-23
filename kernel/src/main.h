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
#include "lists/lists.h"
#include "threads/create_threads.h"

void initialize_global_vars();
void create_servers_cpu();
void create_server_io();

#endif