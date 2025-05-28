#include "main.h"

int main(int argc, char* argv[]) {

    t_config* config_file = init_config("io.config");
    t_io_config io_config = init_io_config(config_file);
    init_logger("io.log", "[IO]", io_config.LOG_LEVEL);
    log_info(get_logger(), "Starting up IO connections...");
    
    int kernel_socket;
    void* args[] = { &kernel_socket, (void*) io_config.PUERTO_KERNEL, (void*) io_config.IP_KERNEL };
    
    create_kernel_connection(args);

    // El primer parámetro argv[0] siempre es para el nombre del programa

    waiting_requests(kernel_socket, argv[1]);
    shutdown_io(io_config, config_file, argv[1]);

    return 0;
}

void waiting_requests(int kernel_socket, char* id_IO){
    while(1){
        t_package* package = recv_package(kernel_socket);
        t_request_io* request = (t_request_io *) read_io_operation_request(package);
        if (io_busy) {
            LOG_DEBUG("OJO, estoy ocupado procesando una operación de I/O.");
            package_destroy(package);
            free(request);
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

void* processing_operation(void* arg) {
    t_request_io* io = (t_request_io*) arg;

    log_info(get_logger(), "## PID: %d - Inicio de IO - Tiempo: %d", io->pid, io->sleep_time);
    
    usleep(io->sleep_time);
    
    log_info(get_logger(), "## PID: %d - Fin de IO", io->pid);
    
    LOG_DEBUG("Estoy libre [%s]", io->device_name);
    
    // RESPONSE
    send_io_operation_completed(io->kernel_socket, io->device_name);
    
    pthread_mutex_lock(&busy_mutex);
    io_busy = false;
    pthread_mutex_unlock(&busy_mutex);

    free(io);
    return NULL;
}