#include "main.h"

int main(int argc, char* argv[]) {

    t_config* config_file = init_config("io.config");
    t_io_config io_config = init_io_config(config_file);
    init_logger("io.log", "[IO]", io_config.LOG_LEVEL);

    log_info(get_logger(), "Starting up IO connections...");

    //=================INIT CONEXIONES===============
    int fd_conexion_IO = create_connection(io_config.PUERTO_KERNEL, io_config.IP_KERNEL);
    init_handshake(argv, fd_conexion_IO);

    waiting_requests(fd_conexion_IO);
    shutdown_io(io_config, config_file);

    return 0;

}

void init_handshake(char* id_IO, int socket_client){
    send(socket_cliente, id_IO, strlen(id_IO)+1, 0);

    t_package* package = safe_malloc(sizeof(t_package));
    package->opcode = HANDSHAKE;
    package->buffer = buffer_create(sizeof(t_buffer));
    buffer_add_string(buffer, strlen(id_IO)+1,id_IO);
    
    int bytes = package->buffer->size + sizeof(int);
    void* stream = serializar(package, bytes);
}

void* serializar(t_package* package, int bytes){
    void *stream = malloc(bytes);
}

void waiting_requests(int socket){
    while(1){
        int32_t tiempo;
        int32_t resultOK = 0;
        recv(socket, &tiempo, sizeof(int32_t), MSG_WAITALL);
        // TODO: deserealizar
        log_info(get_logger(), "## PID: %d - Inicio de IO - Tiempo: %d", 1,tiempo);
        usleep(tiempo);
        // Finalización de la operación I/O
        log_info(get_logger(), "## PID: %d - Fin de IO", 1);
        send(socket, &resultOK, sizeof(int32_t), 0);
    }
}