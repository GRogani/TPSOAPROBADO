#include "main.h"



int main(int argc, char* argv[]) {

    t_config* config_file = init_config("memoria.config");
    t_memoria_config memoria_config = init_memoria_config(config_file);
    init_logger("memoria.log", "[MEMORIA]", memoria_config.LOG_LEVEL);

    log_info(get_logger(), "Starting up memoria connections...");

    int server_socket = create_server(memoria_config.PUERTO_ESCUCHA);

    int client_socket = accept_connection(server_socket); // TODO: usar hilos




    shutdown_memoria(memoria_config ,config_file);

    return 0;
}
