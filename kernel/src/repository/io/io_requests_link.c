#include "io_requests_link.h"

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
    t_io_requests_link *io_request = dictionary_get(get_io_requests_link_dict(), device_name);
    return io_request;
}

void* create_io_request_link(char* device_name) {
    t_io_requests_link* io_request = safe_malloc(sizeof(t_io_requests_link));
    if(io_request == NULL) {
        return NULL;
    }

    t_queue* io_requests_queue = queue_create();
    io_request->io_requests_queue = io_requests_queue;
    initialize_repository_io_requests_queue(&io_request->io_requests_queue_semaphore);

    dictionary_put(get_io_requests_link_dict(), device_name, io_request);

    return io_request;
}

void unlock_io_requests_link() {
    sem_post(&sem_io_requests);
}