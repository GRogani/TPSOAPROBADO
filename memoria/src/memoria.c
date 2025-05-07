#include "memoria.h"

t_memoria_config memoria_config;

int main(int argc, char* argv[]) 
{

    /* ---------------- CONFIG ---------------- */

    t_config *config_file = init_config("memoria.config");
    memoria_config = init_memoria_config(config_file);
    init_user_memory(memoria_config.TAM_MEMORIA);

    /* ---------------- LOGGER ---------------- */
    init_logger("memoria.log", "Memoria", memoria_config.LOG_LEVEL);
    
    /* ---------------- CONEXIONES ---------------- */
    pthread_t listener_thread;
    while(create_server_thread(&listener_thread) != 0);
    


    pthread_join(listener_thread, NULL);
    close(listener_thread);
    shutdown_memoria(config_file);
   
    return 0;

}
