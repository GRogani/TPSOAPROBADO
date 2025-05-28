#include "kernel_connection.h"

int create_kernel_connection(char* puerto, char* ip) 
{

    int connection_socket;
    do {
        connection_socket = create_connection(puerto, ip);
        if (connection_socket != -1)
        {
            log_info(get_logger(), "Connection to Kernel established: %s::%s", ip, puerto);
        } else 
        {
            log_error(get_logger(), "Connection to Kernel failed: %s::%s", ip, puerto);
            log_info(get_logger(), "Retrying connection in 3 seconds");
            sleep(3);
        }
    } while (connection_socket == -1);


    return connection_socket;
}
