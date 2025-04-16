#include "main.h"

int main(int argc, char* argv[]) {
    
    //=================INIT CONFIG/LOGGER===============

    t_config *config_file = init_config("kernel.config");
    t_kernel_config kernel_config = init_kernel_config(config_file);

    init_logger("kernel.log", "[Main]", kernel_config.log_level);

    //=================INIT CONEXIONES===============
    

    int socket_server = create_server(kernel_config.port);

    int socket_client = accept_connection(socket_server);

    shutdown_kernel(kernel_config, config_file);

    return 0;
}
