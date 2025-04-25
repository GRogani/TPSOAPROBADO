#include "memoria.h"

int main(int argc, char* argv[]) {

    /* ---------------- CONFIG ---------------- */

   t_config *config_file = init_config("memoria.config");
   t_memoria_config memoria_config = init_memoria_config(config_file);


    /* ---------------- LOGGER ---------------- */
    init_logger("memoria.log", "Memoria", memoria_config.LOG_LEVEL);
    
    /* ---------------- CONEXIONES ---------------- */
    int socket_server = create_server(memoria_config.PUERTO_ESCUCHA);

    int socket_client = accept_connection(socket_server);

    shutdown_memoria(memoria_config, config_file);
   
    return 0;

}
