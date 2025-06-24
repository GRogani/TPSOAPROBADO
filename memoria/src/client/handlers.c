#include "handlers.h"

extern t_memoria_config memoria_config;

int create_server_thread(pthread_t* listener_thread)
{
    int err = pthread_create(listener_thread, NULL, client_listener, NULL);
    if (err != 0) {
        LOG_ERROR("Listener thread creation failed.");
        return err;
    }

    return 0;
}

void* client_listener(void* arg) {
    int server_fd = create_server(memoria_config.PUERTO_ESCUCHA);
    LOG_INFO("Memoria server listening on port: %s", memoria_config.PUERTO_ESCUCHA);

    while (1) {
        int client_fd = accept_connection("MEMORY SERVER", server_fd);
        if (client_fd < 0) continue;


        pthread_t handler_thread;
        pthread_create(&handler_thread, NULL, client_handler, (void*)&client_fd);
        pthread_detach(handler_thread);
    }

    return NULL;
}

void* client_handler(void* client_fd_ptr) {
    int client_fd = *(int*)client_fd_ptr;

    t_package* package;
    
    while (1) {
        package = recv_package(client_fd);
        if (package == NULL) {
            LOG_INFO("Client disconnected: %d", client_fd);
            close(client_fd);
            return NULL;
        }

        switch (package->opcode) {
            case FETCH:
                get_instruction_request_handler(client_fd, package);
                break;

            case LIST_INSTRUCTIONS:
                get_instruction_request_handler(client_fd, package); // cambiar
                break;

            case GET_FREE_SPACE:
                get_free_space_request_handler(client_fd);
                break;

            case INIT_PROCESS:
                init_process_request_handler(client_fd, package);
                break;

            case UNSUSPEND_PROCESS:
                LOG_INFO("UNSUSPEND_PROCESS received, functionality to be implemented.");
                break;

            case SWAP:
                LOG_INFO("SWAP received, functionality to be implemented.");
                break;

            case KILL_PROCESS:
                delete_process_request_handler(client_fd, package);
                break;

            default:
                LOG_WARNING("Unknown Opcode received: %d from client %d", package->opcode, client_fd);
                close(client_fd);
                destroy_package(package);
                return NULL;
        }

        destroy_package(package);
    }
    return NULL;
}