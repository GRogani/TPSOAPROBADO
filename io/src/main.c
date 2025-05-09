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

    // El primer parámetro argv[0] siempre es para el nombre del programa
    send_handshake(kernel_socket, argv[1]);
    waiting_requests(kernel_socket, argv[1]);
    shutdown_io(io_config, config_file);

    return 0;

}

void waiting_requests(int kernel_socket, char* id_IO){
    while(1){
        t_package* package; //= safe_malloc(sizeof(t_package));
        package = recv_package(kernel_socket);
        t_IO* io = safe_malloc(sizeof(t_IO));
        // TODO: en el struct no estoy pasando el ID del IO
        io = read_IO_operation_request(package);

        // TODO: cómo manejo el caso de de que llegue un msj en un sleep???
        pthread_t t;
        int err = pthread_create(&t, NULL, processing_operation, io);
        if(err) {
            log_error(get_logger(), "Failed to create detachable thread for PROCESSING I/O OPERATION.");
            exit(EXIT_FAILURE);
        }
        LOG_DEBUG("Se procesó la operación de I/O Correctamente.");
        pthread_join(t, NULL);

        send_IO_operation_completed(kernel_socket, id_IO);
        package_destroy(package);
    }
}

void* processing_operation(void* io) {
    // uint32_t* vec = (uint32_t*) args;
    log_info(get_logger(), "## PID: %d - Inicio de IO - Tiempo: %d", ((t_IO*) io)->pid, ((t_IO*) io)->time);
    usleep(((t_IO*) io)->time);
    log_info(get_logger(), "## PID: %d - Fin de IO", ((t_IO*) io)->pid);
    pthread_exit(0);
    return NULL;
}