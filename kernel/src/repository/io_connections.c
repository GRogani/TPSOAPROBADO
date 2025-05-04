#include "io_connections.h"

static sem_t sem_io_connections;

bool initialize_repository_io_connections() {
    if (sem_init(&sem_io_connections, 0, 1) != 0) {
        LOG_ERROR("sem_init for IO_CONNECTIONS list failed");
        exit(EXIT_FAILURE);
    }
}

bool destroy_repository_io_connections() {
    sem_destroy(&sem_io_connections);
}

void lock_io_connection_list() {
    sem_wait(&sem_io_connections);
}

void* find_and_lock_io_connection_list_by_socket(int socket) {
    sem_wait(&sem_io_connections);
    
    bool socket_matches(void* ptr) {
	    t_io_connection* connection = (t_io_connection*) ptr;
	    return connection->socket_id == socket;
	};

    void* el_found = list_find(get_io_connections_list(), socket_matches);

    return el_found;
}

t_list* find_and_lock_io_connections_list_by_device_name(char* device_name) {
    sem_wait(&sem_io_connections);

    t_list* connections_found_list = list_create();

    void build_list(void* ptr) {
	    t_io_connection* connection = (t_io_connection*) ptr;

        if(strcmp(connection->device_name, device_name) == 0) {
	        list_add(connections_found_list, connection->socket_id);
        }
	}

	list_iterate(get_io_connections_list(), build_list);
    
    return connections_found_list;
}

void create_io_connection(int socket, char* device_name) {
    t_io_connection* connection = malloc(sizeof(t_io_connection));
    connection->socket_id = socket;
    connection->device_name = device_name;

    list_add(get_io_connections_list(), connection);
}

void connection_destroyer(void* ptr) {
    t_io_connection* connection = (t_io_connection*) ptr;
    free(connection->device_name);
    free(connection);
}

void delete_io_connection(int socket) {

    bool socket_matches(void* ptr) {
        t_io_connection* connection = (t_io_connection*) ptr;
	    return connection->socket_id == socket;
    }

    list_remove_and_destroy_by_condition(get_io_connections_list(), socket_matches, connection_destroyer);
}

void unlock_io_connections_list() {
    sem_post(&sem_io_connections);
}