
#include "cpu_server.h"

void* cpu_server_handler(void* args) {
    extern t_kernel_config kernel_config;
    int socket_server_dispatch = create_server(kernel_config.cpu_dispatch_port);
    log_info(get_logger(), "CPU dispatch server available on port %s", kernel_config.cpu_dispatch_port);

    int socket_server_interrupt = create_server(kernel_config.cpu_interrupt_port);
    log_info(get_logger(), "CPU interrupt server available on port %s", kernel_config.cpu_interrupt_port);

    while (1) {
        int socket_dispatch_connection = accept_connection(socket_server_dispatch);
        if(socket_dispatch_connection == -1) {
            log_error(get_logger(), "Error accepting dispatch connection");
            
        }
        else{

        }

        int socket_interrupt_connection = accept_connection(socket_server_interrupt);
        if(socket_dispatch_connection == -1) {
            log_error(get_logger(), "Error accepting interrupt connection");
            
            close(socket_dispatch_connection);
            
            break;
        }

        log_info(get_logger(), "CPU connected successfully. Adding connection to list of connected CPUs");
        
        int err = add_cpu_connection(socket_dispatch_connection, socket_interrupt_connection);
        if(err) {
            log_error(get_logger(), "Error adding connections to list");
            break;
        }

        log_info(get_logger(), "CPU connection added to the list!");
        log_info(get_logger(), "Creating thread to handle connection of the CPU with client dispatch: %d. interrupt: %d", socket_dispatch_connection, socket_interrupt_connection);
        
    }
    
    return 0;
}

int add_cpu_connection(int socket_dispatch, int socket_interrupt) {
    t_cpu_connection *cpu_connection = malloc(sizeof(t_cpu_connection));

    if(cpu_connection == NULL) {
        close(socket_dispatch);
        close(socket_interrupt);
        return 0;
    }

    cpu_connection->dispatch_socket_id = socket_dispatch;
    cpu_connection->interrupt_socket_id = socket_interrupt;

    list_add(get_cpu_connections_list(), cpu_connection);

    return 1;
}