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
                unsuspend_process_request_handler(client_fd, package);
                break;

            case SWAP:
                swap_request_handler(client_fd, package);
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

void unsuspend_process_request_handler(int client_fd, t_package* package) {
    uint32_t pid = read_swap_package(package); // Reuse swap_package for PID extraction
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("UNSUSPEND_PROCESS: Proceso PID %u no encontrado.", pid);
        return;
    }
    proc->is_suspended = false;
    LOG_INFO("UNSUSPEND_PROCESS: Proceso PID %u marcado como no suspendido.", pid);
    // TODO: Traer páginas de swap si es necesario
}

void swap_request_handler(int client_fd, t_package* package) {
    uint32_t pid = read_swap_package(package);
    LOG_INFO("SWAP: Solicitud de swap para PID %u (funcionalidad a implementar).", pid);
    // TODO: Implementar lógica de swap real
}