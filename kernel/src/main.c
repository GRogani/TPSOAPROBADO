#include <main.h>

t_kernel_config kernel_config;  
                                
int main(int argc, char* argv[]) {

    t_config* config = init_config("kernel.config");

    kernel_config = init_kernel_config(config);

    initialize_global_lists(); 

    initialize_global_semaphores();

    init_logger("kernel.log", "[Main]", kernel_config.log_level);

    pthread_t io_server_hanlder;
    pthread_t cpu_server_hanlder;
   
    create_servers_threads(&io_server_hanlder, &cpu_server_hanlder);

    // el cpu se crea y una vez que se aprieta enter, se cierra la escucha.
    process_enter(cpu_server_hanlder);

    pthread_join(io_server_hanlder, NULL);

    close(io_server_hanlder);          
    close(cpu_server_hanlder); 

    shutdown_hook(config);  

    return 0;

}

void process_enter(pthread_t cpu_server_hanlder)
{
    printf("Presione Enter para comenzar...\n");
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);

    finish_cpu_server(cpu_server_hanlder); // hacemos que el while deje de correr para siempre.

    wait_cpu_connected(); // esperamos a que termine de conectarse el cpu si es que se estaba conectando.

    // aca deberiamos hacer algo para cerrar el server de escucha de cpu.

    // TODO: validar si hay algun cpu conectado. Si no hay ninguno, tirar error y salir.

    // ejecutamos a mano la syscall init_proc para el proceso 0
    log_info(get_logger(), "Ejecutando syscall init_proc para el proceso 0");
}
