#include "io_client.h"

void* handle_io_client(void* socket)
{
    t_package* package = safe_malloc(sizeof(t_package));
    while (1)
    {
        package = recv_package(*client_socket);
        if(package != NULL)
        {
            case HANDSHAKE:
            {
                case HANDSHAKE: {
                    process_handshake(package, *client_socket);
                    break;
                }
                case IO_COMPLETION:
                {
                    process_io_completion(package, *client_socket);
                    break;
                }
                default:
                {
                    log_error(get_logger(), "Unknown opcode %d from IO device", package->opcode);
                    package_destroy(package);
                    pthread_exit(0);
                    close(*client_socket); 
                    break;
                }
            }
        } else {
            lock_io_connections();
            
            t_io_connection *io_connection = (t_io_connection *)find_io_connection_by_socket(*client_socket);
            if(io_connection == NULL) {
                log_error(get_logger(), "Failed to find IO connection by socket");
                close(*client_socket);
                pthread_exit(0);
                return;
            }
            
            log_error(get_logger(), "Client disconnected %s", io_connection->device_name);

            // TODO: handle closed connection
            close(*client_socket);

            unlock_io_connections();
            break;
        }
    }
}

void process_handshake(t_package* package, int socket) {
    log_info(get_logger(), "Processing HANDSHAKE from client");
    char* device_name = read_io_handshake(package);

    package_destroy(package);

    t_handshake_thread_args* thread_args = safe_malloc(sizeof(t_handshake_thread_args));
    thread_args->client_socket = socket;
    thread_args->device_name = device_name;

    pthread_t io_client_thread;
    int err_io_client = pthread_create(&io_client_thread, NULL, handsake, thread_args);
    if (err_io_client != 0) 
    {
        log_error(get_logger(), "Failed to create IO client HANDSHAKE thread");
    }
    pthread_detach(io_client_thread);
}

void process_io_completion(t_package *package, int socket)
{
    log_info(get_logger(), "Processing IO_COMPLETION from IO device");
    int pid = read_io_completion(package);

    package_destroy(package);

    t_handshake_thread_args *thread_args = safe_malloc(sizeof(t_handshake_thread_args));
    thread_args->client_socket = socket;
    thread_args->pid = pid;

    pthread_t io_client_thread;
    int err_io_client = pthread_create(&io_client_thread, NULL, handsake, thread_args);
    if (err_io_client != 0)
    {
        log_error(get_logger(), "Failed to create IO client HANDSHAKE thread");
    }
    pthread_detach(io_client_thread);
}