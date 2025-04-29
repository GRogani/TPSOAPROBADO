#include "io_client.h"

void* handle_io_client(void* socket)
{
    int* client_socket = (int*)socket;
    t_package* package;
    while (1)
    {
        package = recv_package(*client_socket);

        switch(package->opcode)
        {
            case HANDSHAKE: //func1();
                break;

            default:
            {
                pthread_exit(0);
                close(*client_socket); 
                break;
            }
        }
    }
}