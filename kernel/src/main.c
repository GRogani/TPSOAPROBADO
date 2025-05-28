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
    process_enter();

    pthread_join(io_server_hanlder, NULL);

    close(io_server_hanlder);          
    close(cpu_server_hanlder); 

    shutdown_hook(config);  

    return 0;

}

void process_enter()
{
    printf("Presione Enter para comenzar...\n");
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);

    printf("FINALIZANDO CPU SERVERS, ARRANCANDO PLANIFICACION...\n");

    finish_cpu_server(); // hacemos que el while deje de correr para siempre.

    wait_cpu_connected(); // esperamos a que el thread nos notifique que terminó de correr y limpiar todo antes de cerrar.

    // TODO: validar si hay algun cpu conectado. Si no hay ninguno, tirar error y salir.

    // ejecutamos a mano la syscall init_proc para el proceso 0
    log_info(get_logger(), "Ejecutando syscall init_proc para el proceso 0");
}
