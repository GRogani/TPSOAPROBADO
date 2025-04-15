#include "handle-io-client.h"

void handle_io_client(int socket_client)
{
    while (1)
    {
        // recibir el buffer
        // obtener la info del buffer

        t_io_opcode opcode = HANDSHAKE;
        switch (opcode)
        {
            case HANDSHAKE:
            {
                // add connection to list
                break;
            }

            default:
            {
                log_error(); 
                break;
            }
        }
    }
}