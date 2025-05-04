#include "io_client.h"

void* handle_io_client(void* socket)
{
    int* client_socket = (int*)socket;
    t_package* package;
    while (1)
    {
        package = recv_package(*client_socket);
        if(package)
        {
            switch(package->opcode)
            {
                case HANDSHAKE: {
                    process_handshake(package, *client_socket);
                    break;
                }
                default:
                {
                    pthread_exit(0);
                    close(*client_socket); 
                    break;
                }
            }
        }
    }
}

void process_handshake(t_package* package, int socket) {
    char* device_name = read_handshake(package);
    package_destroy(package); // ESTO NO DEBERIA ELIMINAR EL DEVICE_NAME PORQUE ES OTRO ESPACIO DE MEMORIA

    t_handshake_thread_args* thread_args = malloc(sizeof(t_handshake_thread_args));
    if(thread_args == NULL) {
        // aca que deberiamos hacer? cerrar la conexion? es un error bastante critico que no podamos asignar memoria.
        log_error(get_logger(), "Failed to alloc memory for thread_args on io_client handshake");
        free(device_name);
        return;
    }

    thread_args->client_socket = socket;
    thread_args->device_name = device_name;

    pthread_t io_client_thread;
    int err_io_client = pthread_create(&io_client_thread, NULL, handsake, &thread_args);
    if (err_io_client != 0) 
    {
        log_error(get_logger(), "Failed to create IO client HANDSHAKE thread");
    }    
}