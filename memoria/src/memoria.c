#include "memoria.h"

t_memoria_config memoria_config;

int main(int argc, char* argv[]) {

    /* ---------------- CONFIG ---------------- */

   t_config *config_file = init_config("memoria.config");
   memoria_config = init_memoria_config(config_file);


    /* ---------------- LOGGER ---------------- */
    logger = logger(memoria_config);

    /* ----------------HILOS DE CONEXIONES ---------------- */
    int socket_server = create_server(memoria_config.PUERTO_ESCUCHA);
    while (1) {
        pthread_t thread;
        int *fd_conexion_ptr = malloc(sizeof(int));
        *fd_conexion_ptr = accept(socket_server, NULL, NULL);
        pthread_create(&thread, NULL, (void*) atender_cliente, fd_conexion_ptr);
        pthread_detach(thread);
   }
    //int socket_client = accept_connection(socket_server);

    shutdown_memoria(memoria_config, config_file);
    free(logger);
   
    return 0;

}


void manejar_cliente(int socket_cliente) {
    OPCODE codigo;

    recv(socket_cliente, &codigo, sizeof(OPCODE), 0);

    switch (codigo) {
        case HANDSHAKE:
            printf("Recibido HANDSHAKE\n");
            break;
        /*
        case KERNEL:
            log_trace(logger,"MEMORIA-KERNEL || PID RECIBIDO: %d",pid);
            printf("Recibido HANDSHAKE\n");
            break;
        case CPU:
        printf("Recibido HANDSHAKE\n");
        break;
        */
        case OBTENER_ESPACIO_LIBRE:
            return_esapcioMemoria(socket_cliente);    
            break;
        default:
            printf("Opcode desconocido: %u\n", codigo);
            break;
    }
}


t_log* logger(t_memoria_config config){

    t_log* log = NULL;
    t_log_level level = log_level_from_string(memoria_config.LOG_LEVEL); 
    log = log_create("memoria.log", "Memoria", 1, level);
    
    return log;
}

void return_esapcioMemoria(int socket_cliente){
    int32_t mem_disponible = memoria_config.TAM_MEMORIA;
    send(socket_cliente, &mem_disponible, sizeof(int32_t), 0);
}  