#include <config/config.h>
#include <utils/config/config.h>
#include <utils/logger/logger.h>
#include <utils/socket/server.h>

t_log* logger;

int main(int argc, char* argv[]) {
    t_kernel_config* kernel_config = malloc(sizeof(t_kernel_config));

    if (kernel_config == NULL) {
        printf("Malloc failed");
        exit(EXIT_FAILURE);
    }

    t_config* config = init_and_validate(kernel_config);

    t_log* logger = init_logger(
        "kernel.log",
        "[Main]",
        kernel_config->log_level
    );

    log_info(logger, "Creating server...");
    int port = 3001;
    int socket_server = create_server(port);

    shutdownHook(kernel_config, config, logger);

    return 0;
}

void shutdownHook(t_kernel_config* kernel_config, t_config* config, t_log* logger) {
    free(kernel_config);
    config_destroy(config);
    log_destroy(logger);
}
