#include "handshake.h"

void handsake(void* args) {
    // TODO: deberiamos enviar un handshake de retorno?

    t_handshake_thread_args* thread_args = (t_handshake_thread_args*) args;

    lock_io_connection_list();
    lock_io_requests_link();

    // creamos la nueva instancia de conexion
    create_io_connection(thread_args->client_socket, thread_args->device_name);

    // verificar si existe la lista de requests, sino la creamos
    void* request_link = find_io_request_by_device_name(thread_args->device_name);

    if(request_link == NULL) {
        create_io_request_link(thread_args->device_name);
    }
    
    unlock_io_requests_link();
    unlock_io_connections_list();

    if(request_link != NULL) {
        thread_for_process_next_io(socket, thread_args->device_name);
    }

    free(thread_args->device_name);
    free(thread_args);
}

void thread_for_process_next_io(int socket, char* device_name) {
    t_pending_io_thread_args* thread_args = malloc(sizeof(t_pending_io_thread_args));
    if(thread_args == NULL) {
        log_error(get_logger(), "Could not allocate memory for thread args og process_next_io");
        // que hacemos aca? cerramos la conexion?
        return;
    }

    thread_args->client_socket = socket;
    // aca copiamos el valor, porque lo va a liberar el handshake si le pasamos el mismo espacio de memoria
    thread_args->device_name = strdup(device_name);

    pthread_t process_pending_io_thread;
    int err_io_client = pthread_create(&process_pending_io_thread, NULL, process_pending_io, &thread_args);
    if (err_io_client != 0) 
    {
        log_error(get_logger(), "Failed to create PROCESS_PENDING_IO thread");
    }
}