#include "main.h"

void shutdown_memoria();

t_memoria_config memoria_config; // globales porque quiero usarlas en shutdown
t_config *config_file;

int main(int argc, char *argv[])
{
    config_file = init_config("memoria.config");
    memoria_config = init_memoria_config(config_file);
    init_logger("memoria.log", "[MEMORIA]", memoria_config.LOG_LEVEL);
    initialize_memory_semaphores();
    process_manager_init();
    frame_allocation_init(memoria_config);
    swap_manager_init(memoria_config);
    init_user_space(memoria_config.TAM_MEMORIA);

    pthread_t server_thread;
    create_server_thread(&server_thread);

    signal(SIGABRT, shutdown_memoria);
    pthread_join(server_thread, NULL);    

    return 0;
}

void shutdown_memoria()
{
    LOG_INFO("Shutting down memoria...");
    destroy_logger();
    destroy_memory_semaphores();
    destroy_user_space();
    process_manager_destroy();
    config_destroy(config_file);
    
    exit(EXIT_SUCCESS);
}