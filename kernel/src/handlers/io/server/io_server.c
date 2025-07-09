#include "io_server.h"

void* io_server_handler(void* args) {
    extern t_kernel_config kernel_config; // en main

    int socket_server = create_server(kernel_config.io_port);
    LOG_INFO("IO server available on port %s", kernel_config.io_port);

    while (1)
    {
        int socket_client = accept_connection("IO SERVER", socket_server);
        if(socket_client == -1) {
            LOG_ERROR("Error accepting i/o connection");
            break;
        }

        LOG_INFO("I/O connected successfully. creating thread for client %d...", socket_client);
        pthread_t t;
        int err = pthread_create(&t, NULL, handle_io_client, &socket_client);
        if(err) {
            LOG_ERROR("Failed to create detachable thread for I/O server");
            exit(EXIT_FAILURE);
        }
        pthread_detach(t);
    }
    
    return 0;

}