#include "client.h"

int socket;

void* handle_client(int skt) {

    socket = skt;

    t_package* handshake = recv_package(socket);
    if (!handshake || handshake->opcode != HANDSHAKE) {
        LOG_WARN("Invalid or missing handshake.");
        close(socket);
        return NULL;
    }

    uint32_t id_len;
    char* id = buffer_read_string(handshake->buffer, &id_len);
    bool is_kernel = strcmp(id, "KERNEL") == 0;
    bool is_cpu = strcmp(id, "CPU") == 0;
    free(id);
    package_destroy(handshake);

    if (is_kernel) {
        log_info(logger,"## Kernel Conectado - FD del socket: %d", socket);
        handle_kernel();
    }else if(is_cpu){
        handle_cpu();
    }else{
        log_error(logger,"Unknown client type.");
    }


}

void handle_kernel() {
    while (1) {
        t_package* package = recv_package(socket);
        if (!package) {break; }

        switch (package->opcode) {
            case OBTENER_ESPACIO_LIBRE:
                get_free_space(socket);
                break;

            case CREAR_PROCESO:
                create_process(socket, package->buffer);
                break;

            case LISTA_INSTRUCCIONES:
                get_instruction(socket, package->buffer);
                break;

            default:
                log_error(logger,"Kernel sent unknown or unauthorized opcode: %d", package->opcode);
                break;
        }

        package_destroy(package);
    }

}


void handle_cpu() {
    while (1) {
        t_package* package = recv_package(socket);
        if (!package) break;

        switch (package->opcode) {
            case OBTENER_INSTRUCCION:
                get_instruction(socket, package->buffer);
                break;
                
            case LISTA_INSTRUCCIONES:
                get_instruction(socket, package->buffer);
                break;

            default:
                log_error(logger,"CPU sent unknown or unauthorized opcode: %d", package->opcode);
                break;
        }

        package_destroy(package);
    }


}
