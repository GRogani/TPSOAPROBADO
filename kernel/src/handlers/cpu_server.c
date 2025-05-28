
#include "cpu_server.h"

bool should_exit = false;
int socket_server_dispatch = -1;
int socket_server_interrupt = -1;

void *cpu_server_handler(void *args)
{
    extern t_kernel_config kernel_config;

    socket_server_dispatch = create_server(kernel_config.cpu_dispatch_port);
    log_info(get_logger(), "CPU dispatch server available on port %s", kernel_config.cpu_dispatch_port);

    socket_server_interrupt = create_server(kernel_config.cpu_interrupt_port);
    log_info(get_logger(), "CPU interrupt server available on port %s", kernel_config.cpu_interrupt_port);

    while (true)
    {

        if(should_exit) {
            break;
        }

        log_info(get_logger(), "Accepting dispatch connection");
        int socket_dispatch_connection = accept_connection(socket_server_dispatch);
        if (socket_dispatch_connection == -1)
        {
            log_error(get_logger(), "Error accepting dispatch connection");
            
            if(should_exit) {
                break;
            }

            continue;
        }

        if(should_exit) {
            close(socket_dispatch_connection);
            break;
        }

        // si justo apretó el enter aca, podria entrar primero el enter  a ejecutar y la conexion queda abierta para siempre.

        wait_cpu_connected();

        int socket_interrupt_connection = accept_connection(socket_server_interrupt);
        if (socket_interrupt_connection == -1)
        {
            log_error(get_logger(), "Error accepting interrupt connection");
            close(socket_dispatch_connection);

            if(should_exit) {
                break;
            }

            continue;
        }

        if(should_exit) {
            close(socket_dispatch_connection);
            close(socket_interrupt_connection);
            break;
        }

        log_info(get_logger(), "CPU connected successfully. Adding connection to list of connected CPUs");

        char *connection_id = create_cpu_connection(socket_interrupt_connection, socket_dispatch_connection);

        pthread_t t1;
        int errt1 = pthread_create(&t1, NULL, handle_dispatch_client, connection_id);
        if (errt1)
        {
            log_error(get_logger(), "Failed to create detachable thread for dispatch CPU server");
            
            remove_cpu_connection(connection_id);
            close(socket_dispatch_connection);
            close(socket_interrupt_connection);

            if(should_exit) {
                break;
            }

            continue;
        }

        // Do not create thread for interrupt.
        // interruptions should be awaited in the short term scheduler

        pthread_detach(t1);
    }

    if (socket_server_dispatch != -1) {
        close(socket_server_dispatch);
    }
    if (socket_server_interrupt != -1) {
        close(socket_server_interrupt);
    }
    log_info(get_logger(), "CPU server finished");

    signal_cpu_connected();

    return 0;
}

void finish_cpu_server() {
    should_exit = true;
    
    // Shutdown and close server sockets to interrupt any blocking accept() calls
    if (socket_server_dispatch != -1) {
        shutdown(socket_server_dispatch, SHUT_RDWR);
        close(socket_server_dispatch);
        socket_server_dispatch = -1;
    }
    if (socket_server_interrupt != -1) {
        shutdown(socket_server_interrupt, SHUT_RDWR);
        close(socket_server_interrupt);
        socket_server_interrupt = -1;
    }
}