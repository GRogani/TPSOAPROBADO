#include "io_connections.h"
#include <stdlib.h>

static sem_t sem_io_connections;

void initialize_repository_io_connections() {
    if (sem_init(&sem_io_connections, 0, 1) != 0) {
        LOG_ERROR("sem_init for IO_CONNECTIONS list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_io_connections() {
    sem_destroy(&sem_io_connections);
}

void lock_io_connections() {
    sem_wait(&sem_io_connections);
}

void* find_io_connection_by_socket(int socket) {
    char *stringified_socket = string_itoa(socket);
    
    t_io_connection* element = dictionary_get(get_io_connections_dict(), stringified_socket);

    free(stringified_socket);
    return element;
}

t_list* find_io_connection_by_device_name(char* device_name) {
    t_list* connections_list = dictionary_elements(get_io_connections_dict());

    bool filter_by_device_name(void* ptr) {
        t_io_connection* connection = (t_io_connection*) ptr;
        return strcmp(connection->device_name, device_name) == 0;
    }

    t_list* connections_found_list = list_filter(connections_list, filter_by_device_name);
    list_destroy(connections_list); // Destroy the intermediate list to avoid memory leaks
    
    return connections_found_list;
}

void* find_free_connection_from_device_name(char *device_name)
{
    t_list *connections_list = dictionary_elements(get_io_connections_dict());

    bool find_free_connection(void* ptr) {
        t_io_connection* connection = (t_io_connection*) ptr;
        return strcmp(connection->device_name, device_name) == 0 && connection->current_process_executing == -1;
    }

    void* connection_found = list_find(connections_list, find_free_connection);
    list_destroy(connections_list);

    return connection_found;
}

void create_io_connection(int socket, char* device_name) {
    t_io_connection* connection = safe_malloc(sizeof(t_io_connection));

    connection->device_name = device_name; // utilizamos el mismo puntero que el dto, asi que no lo liberamos.
    connection->current_process_executing = -1;

    char* stringified_socket = string_itoa(socket);

    dictionary_put(get_io_connections_dict(), stringified_socket, connection);
    free(stringified_socket);
}

void update_io_connection_current_processing(int socket, int process_id) {
    char* stringified_socket = string_itoa(socket);
    t_io_connection* connection = dictionary_get(get_io_connections_dict(), stringified_socket);
    free(stringified_socket);

    if (connection != NULL) {
        connection->current_process_executing = process_id;
    }
}

void connection_destroyer(void* ptr) {
    t_io_connection* connection = (t_io_connection*) ptr;
    free(connection->device_name);
    free(connection);
}

void delete_io_connection(int socket) {
    char* stringified_socket = string_itoa(socket);
    t_io_connection* connection = dictionary_remove(get_io_connections_dict(), stringified_socket);
    free(stringified_socket);

    if (connection != NULL) {
        connection_destroyer(connection);
    }
}

void unlock_io_connections() {
    sem_post(&sem_io_connections);
}

t_io_connection_status get_io_connection_status_by_device_name(char *device_name)
{
    t_io_connection_status status = {-1, false, false, NULL};
    t_io_connection_status first_match = {-1, false, false, NULL};
    
    t_list *connections_list = dictionary_elements(get_io_connections_dict());
    t_list *keys_list = dictionary_keys(get_io_connections_dict());
    
    for (int i = 0; i < list_size(connections_list); i++) {
        t_io_connection *connection = (t_io_connection *)list_get(connections_list, i);
        if (strcmp(connection->device_name, device_name) == 0) {
            // Found a connection with the requested device name
            bool is_busy = (connection->current_process_executing != -1);
            char *socket_key = (char *)list_get(keys_list, i);
            int socket_id = atoi(socket_key);
            
            // If this is the first match, save it as fallback
            if (!first_match.found) {
                first_match.found = true;
                first_match.is_busy = is_busy;
                first_match.socket_id = socket_id;
                first_match.connection = connection;
            }
            
            // If this connection is not busy, return it immediately
            if (!is_busy) {
                status.found = true;
                status.is_busy = false;
                status.socket_id = socket_id;
                status.connection = connection;
                
                list_destroy(connections_list);
                list_destroy(keys_list);
                return status;
            }
        }
    }
    
    list_destroy(connections_list);
    list_destroy(keys_list);
    
    // If we found at least one connection but none were free, return the first one
    if (first_match.found) {
        return first_match;
    }
    
    return status; // Not found
}