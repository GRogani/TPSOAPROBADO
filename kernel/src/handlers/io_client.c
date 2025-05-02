#include "io_client.h"

void* handler_io_client(void* socket)
{
    t_package* package = safe_malloc(sizeof(t_package));
    while (1)
    {
        package = recv_package(*(int *) socket);
        switch(package->opcode)
        {
            case HANDSHAKE:
            {
                char* id_IO = read_handshake(package);
                log_info(get_logger(), "IO [%s] conected.", id_IO);
                LOG_DEBUG("Handshake recibido");
                package_destroy(package);
                break;
            }
            case IO:
            {
                char* id_IO = read_IO_operation_completed(package);
                log_info(get_logger(), "IO [%s] operation completed.", id_IO);
                package_destroy(package);
                break;
            }
            default:
                log_error(get_logger(), "INVALID OPCODE or SOCKET FAILURE");
                package_destroy(package);
                pthread_exit(0);
                close(*(int *) socket);
                break;
        }
    }
}