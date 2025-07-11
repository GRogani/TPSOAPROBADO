#include "cpu_server.h"

extern t_kernel_config kernel_config; // en globals.h

int socket_server_dispatch = -1;
int socket_server_interrupt = -1;

int connect_to_cpus(int cpus_quantity)
{

    int err = create_cpu_servers();
    if (err < 0) return err;

    for (int i = 0; i < cpus_quantity; i++)
    {

        int socket_dispatch_connection = accept_connection("DISPATCH SERVER", socket_server_dispatch);
        if (socket_dispatch_connection < 0)
        {
            LOG_ERROR("Failed to accept connection for dispatch CPU server");
            exit(1); // Exit if we cannot accept the interrupt connection
        }

        int socket_interrupt_connection = accept_connection("INTERRUPT SERVER", socket_server_interrupt);
        if (socket_interrupt_connection < 0)
        {
            LOG_ERROR("Failed to accept connection for interrupt CPU server");
            exit(1); // Exit if we cannot accept the interrupt connection
        }

        LOG_INFO("CPU connected successfully. Adding connection to list of connected CPUs");

        char *connection_id = create_cpu_connection(socket_interrupt_connection, socket_dispatch_connection);

        pthread_t t1;
        int errt1 = pthread_create(&t1, NULL, handle_dispatch_client, connection_id);
        if (errt1)
        {
            LOG_ERROR("Failed to create detachable thread for dispatch CPU server");
            
            remove_cpu_connection(connection_id);
            close(socket_dispatch_connection);
            close(socket_interrupt_connection);

            continue;
        }
        pthread_detach(t1);

        // Do not create thread for interrupt.
        // interruptions should be awaited and sent into the short term scheduler
    }

    LOG_INFO("CPU server finished");

    signal_cpu_connected();

    return 0;
}

int create_cpu_servers()
{
    socket_server_dispatch = create_server(kernel_config.cpu_dispatch_port);
    if (socket_server_dispatch > 0)
    {
        LOG_INFO ("CPU dispatch server available on port %s", kernel_config.cpu_dispatch_port);
    }
    else
    {
        LOG_ERROR ("CPU dispatch server creation failed");
        return -1;
    }
    socket_server_interrupt = create_server(kernel_config.cpu_interrupt_port);
    if (socket_server_interrupt > 0)
        LOG_INFO("CPU interrupt server available on port %s", kernel_config.cpu_interrupt_port);
    else
    {
        LOG_ERROR ("CPU interrupt server creation failed");
        return -1;
    }

    return 0;
}
