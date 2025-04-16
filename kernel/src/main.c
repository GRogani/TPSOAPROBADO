#include "config/config.h"
#include "utils/config/config.h"
#include "utils/logger/logger.h"
#include "utils/socket/server.h"
#include "utils/safe_alloc.h"

void shutdown_hook(t_kernel_config kernel_config, t_config* config) {
    log_info(get_logger(), "Shuting down...");
    free(kernel_config.memory_port);
    free(kernel_config.port);
    free(kernel_config.memory_ip);
    config_destroy(config);
    destroy_logger();
}

int main(int argc, char* argv[]) {
    
    //=================INIT CONFIG/LOGGER===============
    //t_kernel_config* kernel_config = safe_malloc(sizeof(t_kernel_config));

    //t_config* config = init_config_and_validate(kernel_config);

    t_config *config_file = init_config("kernel.config");
    t_kernel_config kernel_config = init_kernel_config(config_file);

    init_logger("kernel.log", "[Main]", kernel_config.log_level);

    //=================INIT CONEXIONES===============
    

    int socket_server = create_server(kernel_config.port);

    int socket_client = accept_connection(socket_server);

    shutdown_hook(kernel_config, config_file);

    return 0;
}
