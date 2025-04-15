#include "handle-io-server.h"


void create_io_server() {
    int socket_server = create_server(PUERTO_IO_ESCUCHA);

    while (1)
    {
        int socket_client = accept_connection(socket_server);
        if(socket_client == -1) {
            log_error(logger, "Error accepting i/o connection");
            break;
        }

        log_info("I/O connected successfully. creating thread for client...");

        err = pthread_create(NULL, NULL, handle_io_client, NULL);
        if(err) {
            log_error(logger, "Failed to create detachable thread for I/O server");
            exit(EXIT_FAILURE);
        }
    }
    

}