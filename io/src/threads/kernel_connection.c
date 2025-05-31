#include "kernel_connection.h"

int create_kernel_connection(char* puerto, char* ip) 
{

    int connection_socket;
    do {
        connection_socket = create_connection(puerto, ip);
        if (connection_socket != -1)
        {
            LOG_INFO("Connection to Kernel established: %s::%s", ip, puerto);
        } else 
        {
            LOG_WARNING("Connection to Kernel failed: %s::%s", ip, puerto);
            LOG_WARNING("Retrying connection in 3 seconds");
            sleep(3);
        }
    } while (connection_socket == -1);


    return connection_socket;
}
