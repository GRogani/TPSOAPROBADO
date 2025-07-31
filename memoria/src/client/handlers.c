#include "handlers.h"

extern t_memoria_config memoria_config;

int create_server_thread(pthread_t *listener_thread)
{
    int err = pthread_create(listener_thread, NULL, client_listener, NULL);
    if (err != 0)
    {
        LOG_ERROR("Listener thread creation failed.");
        return err;
    }

    return 0;
}

void *client_listener(void *arg)
{
    int server_fd = create_server(memoria_config.PUERTO_ESCUCHA);
    LOG_INFO("Memoria server listening on port: %s", memoria_config.PUERTO_ESCUCHA);

    while (1)
    {
        int client_fd = accept_connection("MEMORY SERVER", server_fd);
        if (client_fd < 0)
            continue;

        int *client_fd_ptr = safe_malloc(sizeof(int));
        if (client_fd_ptr == NULL)
        {
            LOG_ERROR("Failed to allocate memory for client socket descriptor.");
            close(client_fd);
            continue;
        }
        *client_fd_ptr = client_fd;

        pthread_t handler_thread;
        pthread_create(&handler_thread, NULL, client_handler, (void *)client_fd_ptr);
        pthread_detach(handler_thread);
    }

    return NULL;
}

void *client_handler(void *client_fd_ptr)
{
    int client_fd = *(int *)client_fd_ptr;
    free(client_fd_ptr);

    t_package *package;
    bool kernel_logged = false;

    while (1)
    {
        package = recv_package(client_fd);
        if (package == NULL)
        {
            LOG_INFO("Client disconnected: %d", client_fd);
            close(client_fd);
            return NULL;
        }

        
        if (!kernel_logged)
        {
            switch (package->opcode)
            {
            case INIT_PROCESS:
            case KILL_PROCESS:
            case UNSUSPEND_PROCESS:
            case SWAP:
            case GET_FREE_SPACE:
            case C_DUMP_MEMORY:
                LOG_OBLIGATORIO("## Kernel Conectado - FD del socket: %d", client_fd);
                kernel_logged = true;
                break;
            default:
                break;
            }
        }

        switch (package->opcode)
        {
        case FETCH:
            delay_memory_access();
            get_instruction_request_handler(client_fd, package);
            break;

        case INIT_PROCESS:
            delay_memory_access();
            init_process_request_handler(client_fd, package);
            break;

        case UNSUSPEND_PROCESS:
            unsuspend_process_request_handler(client_fd, package);
            break;

        case SWAP:
            swap_request_handler(client_fd, package);
            break;

        case C_DUMP_MEMORY:
            delay_memory_access();
            dump_memory_request_handler(client_fd, package);
            break;

        case KILL_PROCESS:
            delay_memory_access();
            delete_process_request_handler(client_fd, package);
            break;

        case WRITE_MEMORY:
            write_memory_request_handler(client_fd, package);
            break;

        case READ_MEMORY:
            read_memory_request_handler(client_fd, package);
            break;

        case GET_PAGE_ENTRY:
            delay_memory_access();
            handle_page_walk_request(client_fd, package);
            break;

        default:
            LOG_WARNING("Unknown Opcode received: %s from client %d",opcode_to_string(package->opcode), client_fd);
            close(client_fd);
            destroy_package(package);
            return NULL;
        }

        destroy_package(package);
    }

}
