#include "memoria.h"

extern t_memoria_config memoria_config;
extern t_log* logger;

int main(int argc, char* argv[]) {

    /* ---------------- CONFIG ---------------- */

   t_config *config_file = init_config("memoria.config");
   memoria_config = init_memoria_config(config_file);


    /* ---------------- LOGGER ---------------- */
    init_logger("memoria.log", "Memoria", memoria_config.LOG_LEVEL);
    logger = get_logger();

    /* ----------------HILOS DE CONEXIONES ---------------- */
    int socket_server = create_server(memoria_config.PUERTO_ESCUCHA);
    while (1) {
        pthread_t thread;
        int *fd_conexion_ptr = malloc(sizeof(int));
        *fd_conexion_ptr = accept_connection(socket_server);

        if(*fd_conexion_ptr!= -1){
            pthread_create(&thread, NULL, (void*) handle_client, fd_conexion_ptr);
            pthread_detach(thread);
        }
        
   }

    shutdown_memoria(memoria_config, config_file);
    free(logger);
   
    return 0;

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

