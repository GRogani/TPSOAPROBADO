#include "main.h"
                                
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <script_path> <program_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    extern t_kernel_config kernel_config;  // en globals.h

    t_config *config = init_config("kernel.config");

    kernel_config = init_kernel_config(config);

    configure_scheduling_algorithms();
    
    initialize_global_lists(); 

    initialize_global_semaphores();

    init_logger("kernel.log", "[Kernel]", kernel_config.log_level);

    pthread_t io_server_handler;

    create_io_server_thread(&io_server_handler);
    LOG_INFO("Cantidad de CPUs configuradas: %d", kernel_config.cpu_quantity);
    connect_to_cpus(kernel_config.cpu_quantity); // Levanta el server entran los N cpu y lo baja.

    process_enter(argv[1], atoi(argv[2]));

    pthread_join(io_server_handler, NULL);

    close(io_server_handler);

    shutdown_hook(config);  

    return 0;

}

void process_enter(char* pseudocode_file_name, int32_t program_size)
{   
    wait_cpu_connected(); // esperamos a que el thread nos notifique que terminó de correr y limpiar todo antes de cerrar.
    destroy_cpu_connected_sem();

    printf("Presione Enter para comenzar...\n");
    getchar();

    LOG_INFO("Llamando a syscall init_proc para el proceso 0");
    initialize_root_process(0, program_size, pseudocode_file_name);
}
