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

    // El primer parámetro siempre es para el nombre del programa
    send_handshake(kernel_socket, argv[1]);
    waiting_requests(kernel_socket, argv[1]);
    shutdown_io(io_config, config_file);

    return 0;

}

void waiting_requests(int kernel_socket, char* id_IO){
    while(1){
        
        t_package* package = safe_malloc(sizeof(t_package));
        package = recv_package(kernel_socket);
        /*
        t_IO* io = safe_malloc(sizeof(t_IO));
        io = read_IO_operation_request(package);
        */
        int32_t pid = buffer_read_uint32(package->buffer);
        int32_t tiempo = buffer_read_uint32(package->buffer);
        // Iniciando la operación I/O
        log_info(get_logger(), "## PID: %d - Inicio de IO - Tiempo: %d", pid, tiempo);
        usleep(tiempo);
        // Finalización de la operación I/O
        log_info(get_logger(), "## PID: %d - Fin de IO", pid);
        send_IO_operation_completed(kernel_socket, id_IO);
        package_destroy(package);
    }
}

