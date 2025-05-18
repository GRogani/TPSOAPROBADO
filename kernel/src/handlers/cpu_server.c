
#include "cpu_server.h"

void* cpu_server_handler(void* args) {
    extern t_kernel_config kernel_config;
    int socket_server_dispatch = create_server(kernel_config.cpu_dispatch_port);
    log_info(get_logger(), "CPU dispatch server available on port %s", kernel_config.cpu_dispatch_port);

    int socket_server_interrupt = create_server(kernel_config.cpu_interrupt_port);
    log_info(get_logger(), "CPU interrupt server available on port %s", kernel_config.cpu_interrupt_port);

    while (1) {// TODO: pensar como dejar de aceptar conexiones si ya se apretó el enter.
        int socket_dispatch_connection = accept_connection(socket_server_dispatch);
        if(socket_dispatch_connection == -1) {
            log_error(get_logger(), "Error accepting dispatch connection");
            break;
        }

        int socket_interrupt_connection = accept_connection(socket_server_interrupt);
        if(socket_dispatch_connection == -1) {
            log_error(get_logger(), "Error accepting interrupt connection");
            
            close(socket_dispatch_connection);
            
            break;
        }

        log_info(get_logger(), "CPU connected successfully. Adding connection to list of connected CPUs");
        
        // TODO: crear 2 threads para manejar las conexiones para interrupt y dispatch
        // wait(connection_handshake); -> inicializar el semaforo en -1 cada vez que se espera una conexion de la cpu, de esa forma, se libera cuando se emite 2 veces
        // todo: agregar la cpu a la lista de conexiones
        
    }
    
    return 0;
}