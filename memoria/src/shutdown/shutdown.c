#include "shutdown.h"

void shutdown_memoria(t_config* config) {
    log_info(get_logger(), "Shutting down memoria...");

    destroy_user_memory();
    config_destroy(config);
    destroy_logger();
}