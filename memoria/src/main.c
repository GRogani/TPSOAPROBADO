#include "main.h"

t_memoria_config memoria_config;
glb_memory global_memory;

int main(){

    /* ---------------- CONFIG ---------------- */

   t_config *config_file = init_config("memoria.config");
   memoria_config = init_memoria_config(config_file);


    /* ---------------- LOGGER ---------------- */
    init_logger("memoria.log", "Memoria", memoria_config.LOG_LEVEL);
 
    /* ---------------- MEMORIA GLOBAL ---------------- */
    global_memory.processes = list_create();
    LOG_INFO("Global memory initialized");

    /* ----------------HILOS DE CONEXIONES ---------------- */
    pthread_t server_thread;
    create_server_thread(&server_thread);

    pthread_join(server_thread, NULL);
    LOG_INFO("Server thread finished.");
    shutdown_memoria(memoria_config, config_file);

    return 0;

}


