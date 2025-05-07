#include "handshake.h"

void handsake(void* args) {
    // TODO: deberiamos enviar un handshake de retorno?

    t_handshake_thread_args thread_args = (t_handshake_thread_args) args;

    lock_io_connection_list();
    lock_io_requests_link();

    // creamos la nueva instancia de conexion
    create_io_connection(thread_args->client_socket, thread_args->device_name);

    log_info(get_logger(), "Connection created for device %s", thread_args->device_name);

    // verificar si existe la lista de requests, sino la creamos
    void* request_link = find_io_request_by_device_name(thread_args->device_name);

    if(request_link == NULL) {
        log_info(get_logger(), "First time for this device %s, creating link with requests list", thread_args->device_name);
        create_io_request_link(thread_args->device_name);
    }
    
    unlock_io_requests_link();
    unlock_io_connections_list();

    if(request_link != NULL) {
        call_process_pending_io(socket, thread_args->device_name);
    }

    free(thread_args->device_name);
}

void call_process_pending_io(int socket, char* device_name) {
    t_pending_io_args args;
    args->client_socket = socket;
    args->device_name = strdup(device_name);

    process_pending_io(args);
}