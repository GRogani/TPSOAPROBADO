#include "main.h"

int main(int argc, char* argv[]) {

    if (argc < 2)
        argv[1] = "DISCO"; //nombre defualt si no se especiofica

    t_config* config_file = init_config("io.config");
    t_io_config io_config = init_io_config(config_file);
    init_logger("io.log", "[IO]", io_config.LOG_LEVEL);
    LOG_INFO("Starting up %s connections...", argv[1]);
    
    int kernel_socket;
    kernel_socket = create_connection(io_config.PUERTO_KERNEL, io_config.IP_KERNEL);

    send_new_io_package(kernel_socket, argv[1]);

    waiting_requests(kernel_socket, argv[1]);

    shutdown_io(io_config, config_file);

    return 0;
}

void waiting_requests(int kernel_socket, char* id_IO)
{
    t_package* package;
    io_operation_package_data* request;
    while(1)
    {
        package = recv_package(kernel_socket);
        if(package == NULL)
        {
            LOG_INFO("Kernel disconnected, shutting down I/O device.");
            break;
        }

        request = read_io_operation_package(package);

        request->kernel_socket = kernel_socket;
        request->device_name = id_IO;

        processing_operation(request);

        destroy_package(package);
        free(request);
    }
}

void processing_operation(io_operation_package_data* io) 
{
    LOG_OBLIGATORIO("## PID: %d - Inicio de IO - Tiempo: %d", io->pid, io->sleep_time);
    
    usleep(io->sleep_time * 1000); // milliseconds -> microseconds
    
    LOG_OBLIGATORIO("## PID: %d - Fin de IO", io->pid);
    
    LOG_DEBUG("Estoy libre [%s]", io->device_name);
    
    // RESPONSE
    send_io_completion_package(io->kernel_socket, io->device_name, io->pid);
    
    //Desestimado
    // pthread_mutex_lock(&busy_mutex);
    // io_busy = false;
    // pthread_mutex_unlock(&busy_mutex);

}