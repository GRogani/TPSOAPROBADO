#include "io_client.h"

void* handle_io_client(void* socket)
{
    int client_socket = *(int*) socket;
    t_package* package = safe_malloc(sizeof(t_package));
    while (1)
    {
        package = recv_package(client_socket);
        if(package != NULL)
        {
            switch(package->opcode)
            {
                case IO_NEW_DEVICE: {
                    handle_new_device(package, client_socket);
                    break;
                }
                case IO_COMPLETION:
                {
                    process_io_completion(package, client_socket);
                    break;
                }
                default:
                {
                    LOG_ERROR("Unknown opcode %d from IO device", package->opcode);
                    package_destroy(package);
                    pthread_exit(0);
                    close(client_socket); 
                    break;
                }
            }
        } else {
            lock_io_connections();
            
            t_io_connection *io_connection = (t_io_connection *)find_io_connection_by_socket(client_socket);
            if(io_connection == NULL) {
                LOG_ERROR("Failed to find IO connection by socket");
                close(client_socket);
                pthread_exit(0);
                return NULL;
            }
            
            LOG_ERROR("Client disconnected %s", io_connection->device_name);

            // TODO: handle closed connection
            close(client_socket);

            unlock_io_connections();
            break;
        }
    }
}

void handle_new_device(t_package* package, int socket) {
    LOG_INFO("Processing new_device from client");
    char* device_name = read_new_device(package);

    package_destroy(package);

    t_new_device_thread_args* thread_args = safe_malloc(sizeof(t_new_device_thread_args));
    thread_args->client_socket = socket;
    thread_args->device_name = device_name;

    pthread_t io_client_thread;
    int err_io_client = pthread_create(&io_client_thread, NULL, process_new_device, thread_args);
    if (err_io_client != 0) 
    {
        LOG_ERROR("Failed to create IO client new_device thread");
    }
    pthread_detach(io_client_thread);
}

void process_io_completion(t_package *package, int socket)
{
    LOG_INFO("Processing IO_COMPLETION from IO device");
    char* device_name = read_io_operation_completed(package);

    package_destroy(package);

    t_completion_thread_args *thread_args = safe_malloc(sizeof(t_completion_thread_args));
    thread_args->client_socket = socket;
    thread_args->device_name = device_name;

    pthread_t io_client_thread;
    int err_io_client = pthread_create(&io_client_thread, NULL, io_completion, thread_args);
    if (err_io_client != 0)
    {
        LOG_ERROR("Failed to create IO client HANDSHAKE thread");
    }
    pthread_detach(io_client_thread);
}