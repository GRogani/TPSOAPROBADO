
#include "cpu_server.h"

bool should_exit = false;

void *cpu_server_handler(void *args)
{
    extern t_kernel_config kernel_config;

    int socket_server_dispatch = create_server(kernel_config.cpu_dispatch_port);
    log_info(get_logger(), "CPU dispatch server available on port %s", kernel_config.cpu_dispatch_port);

    int socket_server_interrupt = create_server(kernel_config.cpu_interrupt_port);
    log_info(get_logger(), "CPU interrupt server available on port %s", kernel_config.cpu_interrupt_port);

    while (!should_exit)
    {
        log_info(get_logger(), "Accepting dispatch connection");
        int socket_dispatch_connection = accept_connection(socket_server_dispatch);
        if (socket_dispatch_connection == -1)
        {
            log_error(get_logger(), "Error accepting dispatch connection");
            continue;
        }

        // si justo apretó el enter aca, podria entrar primero el enter  a ejecutar y la conexion queda abierta para siempre.

        wait_cpu_connected();

        int socket_interrupt_connection = accept_connection(socket_server_interrupt);
        if (socket_interrupt_connection == -1)
        {
            log_error(get_logger(), "Error accepting interrupt connection");

            close(socket_dispatch_connection);
            signal_cpu_connected();

            continue;
        }

        log_info(get_logger(), "CPU connected successfully. Adding connection to list of connected CPUs");

        char *connection_id = create_cpu_connection(socket_interrupt_connection, socket_dispatch_connection);

        pthread_t t1;
        int errt1 = pthread_create(&t1, NULL, handle_dispatch_client, connection_id);
        if (errt1)
        {
            log_error(get_logger(), "Failed to create detachable thread for dispatch CPU server");
            close(socket_dispatch_connection);
            close(socket_interrupt_connection);
            signal_cpu_connected();
            continue;
        }
        pthread_detach(t1);

        pthread_t t2;
        int errt2 = pthread_create(&t2, NULL, handle_interrupt_client, connection_id);
        if (errt2)
        {
            log_error(get_logger(), "Failed to create detachable thread for interrupt CPU server");
            close(socket_dispatch_connection);
            close(socket_interrupt_connection);
            signal_cpu_connected();
            continue;
        }
        pthread_detach(t2);

        signal_cpu_connected();
    }

    close(socket_server_dispatch);
    close(socket_server_interrupt);
    log_info(get_logger(), "CPU server finished");

    return 0;
}

void finish_cpu_server() {
    should_exit = true;
}