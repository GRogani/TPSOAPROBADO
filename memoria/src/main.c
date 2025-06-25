#include "main.h"

//Globals
t_memoria_config memoria_config;

int main(){

    t_config *config_file = init_config("memoria.config");
    memoria_config = init_memoria_config(config_file);
    init_logger("memoria.log", "Memoria", memoria_config.LOG_LEVEL);
    initialize_memory_semaphores();
    init_user_memory(&memoria_config);
    process_manager_init();
    swap_manager_init(&memoria_config);
    frame_allocation_init();

    pthread_t server_thread;
    create_server_thread(&server_thread);

    pthread_join(server_thread, NULL);
    LOG_INFO("Server thread finished.");

    destroy_memory_semaphores();
    LOG_INFO("Memory semaphores destroyed");

    process_manager_destroy(); // cleanup for process manager and its process list
    shutdown_memoria(memoria_config, config_file);

    return 0;
}


