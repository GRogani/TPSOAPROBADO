#include "io_client.h"

void* handle_io_client(void* socket)
{
    int client_socket = *(int*) socket;
    while (1)
    {
        t_package *package = recv_package(client_socket);
        if(package != NULL)
        {
            switch(package->opcode)
            {
                case NEW_IO: {
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
                    destroy_package(package);
                    break;
                }
            }
        } else {
            io_disconnected(client_socket);
            break;
        }
    }
}

void handle_new_device(t_package* package, int socket) {
    LOG_INFO("Processing new_device from client");
    char* device_name = read_new_io_package(package);

    destroy_package(package);

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
    io_completion_package_data* completion_data = read_io_completion_package(package);

    destroy_package(package);

    t_completion_thread_args *thread_args = safe_malloc(sizeof(t_completion_thread_args));
    thread_args->client_socket = socket;
    thread_args->device_name = completion_data->device_name;
    thread_args->pid = completion_data->pid;

    pthread_t io_client_thread;
    int err_io_client = pthread_create(&io_client_thread, NULL, io_completion, thread_args);
    if (err_io_client != 0)
    {
        LOG_ERROR("Failed to create IO client HANDSHAKE thread");
    }
    pthread_detach(io_client_thread);
}