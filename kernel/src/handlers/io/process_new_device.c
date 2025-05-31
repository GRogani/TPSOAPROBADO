#include "process_new_device.h"

void* process_new_device(void* thread_args)
{    
    t_new_device_thread_args* args = (t_new_device_thread_args*)thread_args;

    lock_io_connections();
    lock_io_requests_link();

    // creamos la nueva instancia de conexion
    create_io_connection(args->client_socket, args->device_name);

    LOG_INFO("Connection created for device %s", args->device_name);

    // verificar si existe el diccionario de requests, sino lo creamos
    void* request_link = find_io_request_by_device_name(args->device_name);

    if(request_link == NULL) {
        LOG_INFO("First time for this device %s, creating link with requests dictionary", args->device_name);
        create_io_request_link(args->device_name);
    }
    
    unlock_io_requests_link();
    unlock_io_connections();

    if(request_link != NULL) {
        call_process_pending_io(args->client_socket, args->device_name);
    }

    // el device_name se utiliza para la connection. se libera cuando la connection se elimina.
    free(args);
}

void call_process_pending_io(int socket, char* device_name) {
    t_pending_io_args args;
    args.client_socket = socket;
    args.device_name = strdup(device_name);

    process_pending_io(args);
}