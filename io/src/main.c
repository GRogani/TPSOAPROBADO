#include "main.h"

int main(int argc, char* argv[]) {
    
    t_config* config_file = init_config("io.config");
    t_io_config io_config = init_io_config(config_file);
    init_logger("io.log", "[IO]", io_config.LOG_LEVEL);

    log_info(get_logger(), "Starting up IO connections...");

    create_connection(io_config.PUERTO_KERNEL, io_config.IP_KERNEL);


    shutdown_io(io_config, config_file);

    return 0;
}
