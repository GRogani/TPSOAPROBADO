#include "io_requests.h"

static sem_t sem_io_requests;

void initialize_repository_io_requests_link() {
    if (sem_init(&sem_io_requests, 0, 1) != 0) {
        LOG_ERROR("sem_init for IO_REQUESTS link failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_io_requests_link() {
    sem_destroy(&sem_io_requests);
}

void lock_io_requests_link() {
    sem_wait(&sem_io_requests);
}

void* find_io_request_by_device_name(char* device_name) {
    bool device_name_matches(void* ptr) {
	    t_io_requests_link* request = (t_io_requests_link*) ptr;
	    return strcmp(request->device_name, device_name) == 0;
	};

    void* el_found = list_find(get_io_requests_link_list(), device_name_matches);

    return el_found;
}

void create_io_request_link(char* device_name) {
    t_io_requests_link* io_requests = malloc(sizeof(t_io_requests_link));
    if(io_requests == NULL) {
        return -1;
    }

    initialize_repository_io_requests_list();
    t_list* requests_list = list_create();

    io_requests->device_name = device_name;
    io_requests->requests_list = requests_list;
}

/*
 * @brief returns the first connection free (not in use) or NULL if there's no connection free
 * should be used with find_and_lock_io_connections_list_by_device_name which returns io sockets for the provided device_name.  Should be used when validating before creating io_request.
 * @returns socket_id as integer if connection found, otherwise, returns -1 if no connection free found
 */
void* get_free_connection(t_list* connections_socket_list, char* device_name) {
    t_io_requests_link* io_request_link = find_io_request_by_device_name(device_name);

    void connections_iterator(void* ptr) {
        int socket_id = (int) ptr;

        bool socket_matches(void* ptr) {
            t_io_request *request = (t_io_request*) ptr;
            return request->socket_id == socket_id;
	    };

        // verificar si se encuentra ese socket en la lista de requests.
        void* el_found = list_find(io_request_link->requests_list, socket_matches);

        // si no se encuentra, no está en uso, podemos devolverlo
        if(el_found == NULL) {
            return socket_id;
        }
    }

    list_iterate(connections_socket_list, connections_iterator);

    return -1;
}

void unlock_io_requests_link() {
    sem_post(&sem_io_requests);
}