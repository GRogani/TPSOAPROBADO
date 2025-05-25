#include <main.h>

t_kernel_config kernel_config;  
                                
int main(int argc, char* argv[]) {

    t_config* config = init_config("kernel.config");

    kernel_config = init_kernel_config(config);

    initialize_global_lists(); 

    init_logger("kernel.log", "[Main]", kernel_config.log_level);

    pthread_t io_server_hanlder;
    pthread_t cpu_server_hanlder;
   
    create_servers_threads(&io_server_hanlder, &cpu_server_hanlder);

    // TODO: el cpu se crea y una vez que se aprieta enter, se cierra la escucha.

    pthread_join(io_server_hanlder, NULL);
    pthread_join(cpu_server_hanlder, NULL);

    close(io_server_hanlder);          
    close(cpu_server_hanlder); 

    shutdown_hook(config);  

    return 0;
}
