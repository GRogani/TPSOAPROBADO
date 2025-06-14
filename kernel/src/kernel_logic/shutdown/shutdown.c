#include "shutdown.h"
#include <stdio.h>
#include <stdlib.h>

void shutdown_hook(t_config *config)
{
    LOG_INFO("Shutting down kernel...");

    destroy_global_lists();

    destroy_global_repositories();

    config_destroy(config);

    destroy_logger(get_logger());

    LOG_INFO("Kernel died gracefuly");

    exit(EXIT_SUCCESS);
}