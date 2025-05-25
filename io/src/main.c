#include "main.h"

int main(int argc, char* argv[]) {
    
    t_config* config_file = init_config("io.config");
    t_io_config io_config = init_io_config(config_file);
    init_logger("io.log", "[IO]", io_config.LOG_LEVEL);

    log_info(get_logger(), "Starting up IO connections...");

    int kernel_socket;
    void* args[] = { &kernel_socket, (void*) io_config.PUERTO_KERNEL, (void*) io_config.IP_KERNEL };
    pthread_t connection_thread;
    pthread_create(&connection_thread, NULL, create_kernel_connection, args);

    pthread_join(connection_thread, NULL);

    // si, ese thread de arriba no hace falta.

    send_handshake(kernel_socket, "IO");

    shutdown_io(io_config, config_file);

    return 0;
}
