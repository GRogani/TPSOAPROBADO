#include "shutdown.h"

void shutdown_cpu(t_cpu_config cpu_config, t_config* config) {
    LOG_INFO("Shutting down CPU...");

    //Agregar...

    config_destroy(config);
    destroy_logger();
}

void shutdown_memoria(t_memoria_config memoria_config, t_config* config) {
    LOG_INFO("Shutting down memoria...");

    //Agregar...

    config_destroy(config);
    destroy_logger();
}

void shutdown_io(t_io_config io_config, t_config* config) {
    LOG_INFO("Shutting down IO...");
    
    config_destroy(config);
    destroy_logger();
}
