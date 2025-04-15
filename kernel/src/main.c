#include <config/config.h>
#include <utils/config/config.h>
#include <utils/logger/logger.h>
#include <utils/socket/server.h>

t_log* logger;

void shutdown_hook(t_kernel_config* kernel_config, t_config* config, t_log* logger) {
    free(kernel_config->memory_port);
    free(kernel_config->port);
    free(kernel_config);
    config_destroy(config);
    log_destroy(logger);
}

int main(int argc, char* argv[]) {
    t_kernel_config* kernel_config = malloc(sizeof(t_kernel_config));

    if (kernel_config == NULL) {
        printf("Malloc failed");
        exit(EXIT_FAILURE);
    }

    t_config* config = init_config_and_validate(kernel_config);

    t_log* logger = init_logger(
        "kernel.log",
        "[Main]",
        kernel_config->log_level
    );

    log_info(logger, "Creating server...");

    int socket_server = create_server(kernel_config->port);
    log_info(logger, "awaiting for new clients...");
    int socket_client = accept_connection(socket_server);

    shutdown_hook(kernel_config, config, logger);

    return 0;
}
