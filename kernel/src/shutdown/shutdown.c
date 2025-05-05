#include "shutdown.h"
#include <stdio.h>
#include <stdlib.h>

void shutdown_hook(t_config* config) {
    log_info(get_logger(), "Shutting down kernel...");

    destroy_global_lists();

    destroy_repository_new();
    destroy_repository_io_connections();
    destroy_repository_io_requests_link();
    destroy_repository_io_requests_list();
    
    config_destroy(config);

    destroy_logger(get_logger());

    LOG_DEBUG("Kernel died gracefuly");
    
    exit(EXIT_SUCCESS);
}