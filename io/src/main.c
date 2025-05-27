#include "main.h"
#include "data_request.h"

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
        t_package* package = recv_package(kernel_socket);
        t_request_IO* request = (t_request_IO *) read_IO_operation_request(package);
        if (io_busy) {
            LOG_DEBUG("OJO, estoy ocupado procesando una operación de I/O.");
            package_destroy(package);
            request_destroy(request);
            continue;
        } else {
            pthread_mutex_lock(&busy_mutex);
            io_busy = true;
            pthread_mutex_unlock(&busy_mutex);
        }
        request->kernel_socket = kernel_socket;
        request->device_name = id_IO;
        pthread_t t;
        int err = pthread_create(&t, NULL, processing_operation, request);
        if(err) {
            log_error(get_logger(), "Failed to create detachable thread for PROCESSING I/O OPERATION.");
            exit(EXIT_FAILURE);
        }
        pthread_detach(t);
        package_destroy(package);
    }
}

void* processing_operation(void* io) {
    log_info(get_logger(), "## PID: %d - Inicio de IO - Tiempo: %d", ((t_request_IO*) io)->pid, ((t_request_IO*) io)->sleep_time);
    usleep(((t_request_IO*) io)->sleep_time);
    log_info(get_logger(), "## PID: %d - Fin de IO", ((t_request_IO*) io)->pid);
    LOG_DEBUG("Estoy libre [%s]", ((t_request_IO*) io)->device_name);
    // RESPONSE
    send_IO_operation_completed(((t_request_IO*) io)->kernel_socket, ((t_request_IO*) io)->device_name);
    
    pthread_mutex_lock(&busy_mutex);
    io_busy = false;
    pthread_mutex_unlock(&busy_mutex);

    request_destroy(((t_request_IO*) io));
    return NULL;
}