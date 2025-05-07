#include "handlers.h"

extern t_memoria_config memoria_config;

int create_server_thread(pthread_t* listener_thread)
{
    int err = pthread_create(listener_thread, NULL, client_listener, NULL);
    if (err != 0) {
        log_error(get_logger(), "Listener thread creation failed.");
        return err;
    }

    return 0;
}

void* client_listener(void* arg) {
    int server_fd;
    do{
        server_fd = create_server(memoria_config.PUERTO_ESCUCHA);
    }while(server_fd < 0);

    log_info(get_logger(), "Memoria server listening on port: %s", memoria_config.PUERTO_ESCUCHA);

    while (1) {
        int client_fd = accept_connection(server_fd);
        if (client_fd < 0) continue;

        pthread_t handler_thread;
        pthread_create(&handler_thread, NULL, client_handler, &client_fd);
        pthread_detach(handler_thread);
    }

    pthread_exit(0);
    close(server_fd);
    return NULL;
}

void* client_handler(void* client_fd_ptr) {
    int client_fd = *(int*)client_fd_ptr;

    t_package* package;
    
    while(1)
    {
        package = recv_package(client_fd);
        if (package == NULL) continue;

        switch (package->opcode) 
        {
            case HANDSHAKE:
                process_handshake(package);
                break;
            default:
                log_warning(get_logger(), "Unknown Opcode");
                package_destroy(package);
                break;
        }
    }
    return NULL;
}
