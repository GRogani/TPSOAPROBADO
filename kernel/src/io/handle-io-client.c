#include "handle-io-client.h"

void handle_io_client(void* socket_IO)
{
    while (1)
    {
        // recibir el buffer
        t_package* package = safe_malloc(sizeof(t_package));
        package = recv_package(*(int *) socket_IO);
        // obtener la info del buffer
        OPCODE opcode = package->opcode;
        switch (opcode)
        {
            case HANDSHAKE:
            {
                char* id_IO = read_handshake(package);
                log_info(get_logger(), "IO [%d] connected.", id_IO);
                // add connection to list
                // list_add(io_connections_list, id_IO);
                package_destroy(package);
                break;
            }
            case IO:
            {
                char* id_IO = send_IO_operation_completed(package);
                log_info(get_logger(), "IO [%d] operation completed.", id_IO);
                package_destroy(package);
            }
            default:
            {
                log_error(get_logger(), "I/O conecction failed."); 
                break;
            }
        }
    }
}