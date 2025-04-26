#include "handle-io-server.h"

void create_io_server() {
    int socket_server = create_server(PUERTO_IO_ESCUCHA);
    log_info(get_logger(), "IO server available on port %s", PUERTO_IO_ESCUCHA);

    while (1)
    {
        int socket_client = accept_connection(socket_server);
        if(socket_client == -1) {
            log_error(get_logger(), "Error accepting i/o connection");
            break;
        }

        log_info(get_logger(), "I/O connected successfully. creating thread for client %d...", socket_client);
        pthread_t t;
        int err = pthread_create(&t, NULL, handle_io_client, socket_client);
        if(err) {
            log_error(get_logger(), "Failed to create detachable thread for I/O server");
            exit(EXIT_FAILURE);
        }
        pthread_join(t, NULL);
        // Caso de Prueba
        send_IO_operation_request(socket_client, 2, 10);
    }
}