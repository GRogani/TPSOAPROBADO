#include "main.h"
                                
int main(int argc, char* argv[]) {

    extern t_kernel_config kernel_config;  // en globals.h

    t_config* config = init_config("kernel.config");

    kernel_config = init_kernel_config(config);

    initialize_global_lists(); 

    initialize_global_semaphores();

    init_logger("kernel.log", "[Kernel]", kernel_config.log_level);

    pthread_t io_server_hanlder;
    pthread_t cpu_server_hanlder;
   
    create_io_server_thread(&io_server_hanlder);

    connect_to_cpus(kernel_config.cpu_quantity); // Levanta el server entran los N cpu y lo baja.

    process_enter(argv[1], atoi(argv[2]));

    pthread_join(io_server_hanlder, NULL);

    close(io_server_hanlder);          

    shutdown_hook(config);  

    return 0;

}

void process_enter(char* pseudocode_file_name, uint32_t program_size)
{   
    wait_cpu_connected(); // esperamos a que el thread nos notifique que terminó de correr y limpiar todo antes de cerrar.
    destroy_cpu_connected_sem();

    printf("Presione Enter para comenzar...\n");
    getchar();

    // TODO: validar si hay algun cpu conectado. Si no hay ninguno, tirar error y salir.

    LOG_INFO("Ejecutando syscall init_proc para el proceso 0");
    handle_init_proc_syscall(0, program_size, pseudocode_file_name);
}
