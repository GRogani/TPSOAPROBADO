#include "shutdown.h"

void shutdown_kernel(t_kernel_config kernel_config, t_config* config) {
    log_info(get_logger(), "Shutting down kernel...");

    free(kernel_config.port);
    free(kernel_config.memory_ip);
    free(kernel_config.memory_port);

    config_destroy(config);
    destroy_logger();
}

void shutdown_cpu(t_cpu_config cpu_config, t_config* config) {
    log_info(get_logger(), "Shutting down CPU...");

    free(cpu_config.IP_MEMORIA);
    free(cpu_config.PUERTO_MEMORIA);
    free(cpu_config.IP_KERNEL);
    free(cpu_config.PUERTO_KERNEL_DISPATCH);
    free(cpu_config.PUERTO_KERNEL_INTERRUPT);
    free(cpu_config.REEMPLAZO_TLB);
    free(cpu_config.REEMPLAZO_CACHE);

    config_destroy(config);
    destroy_logger();
}

void shutdown_memoria(t_memoria_config memoria_config, t_config* config) {
    log_info(get_logger(), "Shutting down memoria...");

    free(memoria_config.PATH_SWAPFILE);
    free(memoria_config.DUMP_PATH);

    config_destroy(config);
    destroy_logger();
}

void shutdown_io(t_io_config io_config, t_config* config) {
    log_info(get_logger(), "Shutting down IO...");

    free(io_config.IP_KERNEL);
    free(io_config.PUERTO_KERNEL);

    config_destroy(config);
    destroy_logger();
}
