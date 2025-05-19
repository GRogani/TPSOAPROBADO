
#include "cpu_server.h"

void* cpu_server_handler(void* args) {
    extern t_kernel_config kernel_config;
    int socket_server_dispatch = create_server(kernel_config.cpu_dispatch_port);
    log_info(get_logger(), "CPU dispatch server available on port %s", kernel_config.cpu_dispatch_port);

    int socket_server_interrupt = create_server(kernel_config.cpu_interrupt_port);
    log_info(get_logger(), "CPU interrupt server available on port %s", kernel_config.cpu_interrupt_port);

    while (1) {
        // Si justo se presiona enter y se estaba agregando la conexion, pasan cosas raras.
        // con esto sincronizamos y solamente se planifica si no se está agregando una nueva conexion.
        wait_cpu_connected();
        int socket_dispatch_connection = accept_connection(socket_server_dispatch);
        if(socket_dispatch_connection == -1) {
            log_error(get_logger(), "Error accepting dispatch connection");
            break;
        }

        int socket_interrupt_connection = accept_connection(socket_server_interrupt);
        if (socket_interrupt_connection == -1)
        {
            log_error(get_logger(), "Error accepting interrupt connection");

            close(socket_dispatch_connection);

            break;
        }

        log_info(get_logger(), "CPU connected successfully. Adding connection to list of connected CPUs");

        // agregamos la cpu a la conexion de cpus
        
        // TODO: crear 2 threads detachables para manejar las conexiones para interrupt y dispatch

        signal_cpu_connected();
    }
    
    return 0;
}