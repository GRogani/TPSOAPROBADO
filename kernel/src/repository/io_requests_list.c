#include "io_requests_list.h"

static sem_t sem_io_requests_list;

bool initialize_repository_io_requests_list() {
    if (sem_init(&sem_io_requests_list, 0, 1) != 0) {
        LOG_ERROR("sem_init for IO_REQUESTS_LIST failed");
        exit(EXIT_FAILURE);
    }
}

bool destroy_repository_io_requests_list() {
    sem_destroy(&sem_io_requests_list);
}

void lock_io_requests_list() {
    sem_wait(&sem_io_requests_list);
}

void create_io_request_element(t_list* list, int process_id, int sleep_time) {
    t_io_request* list_element = malloc(sizeof(t_io_request));
    list_element->pid = process_id;
    list_element->processing = false;
    list_element->socket_id = -1;
    list_element->sleep_time = sleep_time;

    list_add(list, list_element); // la nueva request siempre se envia al final
}

void* find_first_element_for_socket(t_list* list, int socket) {
    bool request_socket_processing_matches(void* ptr) {
	    t_io_request* request = (t_io_request*) ptr;
	    return request->socket_id == socket && request->processing;
	};

    return list_find(list, request_socket_processing_matches);
}

void* find_first_element_without_assign(t_list* list) {
     bool request_not_assigned_matches(void* ptr) {
	    t_io_request* request = (t_io_request*) ptr;
	    return !request->processing;
	};

    return list_find(list, request_not_assigned_matches);
}

void* assign_connection_to_request(void* element_to_assign, int socket)
{
    t_io_request * io_request = (t_io_request *)element_to_assign;
    if (io_request == NULL)
    {
        return io_request;
    }

    io_request->socket_id = socket;
    io_request->processing = true;
    
    return io_request;
}

void unlock_io_requests_list() {
    sem_post(&sem_io_requests_list);
}