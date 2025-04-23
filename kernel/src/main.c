#include <main.h>

t_kernel_config kernel_config;  // esta bien que sea global porque no va a cambiar en ejecucion
                                // siewmpre es mejor pasar por parametro igual
int main(int argc, char* argv[]) {

    t_config* config = init_config("kernel.config");

    kernel_config = init_kernel_config(config);

    initialize_global_lists(); // <--- para esas listas va a haber que aplicar semaforos

    init_logger("kernel.log", "[Main]", kernel_config.log_level);


    pthread_t io_server_hanlder;
    pthread_t cpu_server_hanlder;
    // serian los threads principales
    create_servers_threads(&io_server_hanlder, &cpu_server_hanlder);
    
    //create_client_thread() para memoria o renombrar create_threads y que ahi esten todos
    // o create_main_threads()...
    

    pthread_join(io_server_hanlder, NULL);    // esperamos a que terminen los threads principales   
    pthread_join(cpu_server_hanlder, NULL);   // no tendiran que joinear nunca igual
    close(io_server_hanlder);           // pero en caso de exit dejo eso
    close(cpu_server_hanlder); 
    shutdown_hook(config);         
    // signal(SIGINT, shutdown_hook);
    // signal(SIGTERM, shutdown_hook);
    //while(1) pause(); // TODO: use joinable threads and remove this line. detachable threads could die and you will never notice
    return 0;
}
