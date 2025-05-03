#include "lists.h"


static t_list* io_connections_list;
static t_queue* io_requests_list;
static t_list* cpu_connections_list;
static t_list* new_list;
static t_list* ready_list;
static t_list* exec_list;
static t_list* blocked_list;
static t_list* susp_blocked_list;
static t_list* susp_ready_list;
static t_list* exit_list;

t_list* get_cpu_connections_list()
{
    return cpu_connections_list;
}

t_list* get_new_list()
{
    return new_list;
}

void initialize_global_lists() {

    io_connections_list = list_create();
    cpu_connections_list = list_create();

    new_list = list_create();
    initialize_repository_new();

    ready_list = list_create();
    exec_list = list_create();
    blocked_list = list_create();
    susp_blocked_list = list_create();
    susp_ready_list = list_create();
    exit_list = list_create();

    io_requests_list = queue_create();

    if(
        io_connections_list == NULL || 
        io_requests_list == NULL ||
        cpu_connections_list == NULL ||
        new_list == NULL ||
        ready_list == NULL ||
        exec_list == NULL ||
        blocked_list == NULL ||
        susp_blocked_list == NULL ||
        susp_ready_list == NULL ||
        exit_list == NULL
    ) {
        log_error(get_logger(), "Some of the lists/queues failed to create");
        exit(EXIT_FAILURE);
    }

    
}

void destroy_global_lists(){

    list_destroy_and_destroy_elements(io_connections_list, io_connections_destroyer);
    list_destroy_and_destroy_elements(cpu_connections_list, cpu_connections_destroyer);
    list_destroy_and_destroy_elements(io_requests_list, io_requests_destroyer);

}


void io_connections_destroyer(void *ptr) {
    t_io_connection* connection = (t_io_connection*) ptr;
    free(connection->device_name);
    free(connection);
}

void io_requests_queue_destroyer(void *ptr) {
    t_io_queue* queue = (t_io_queue*) ptr;
    free(queue);
}


void io_requests_destroyer(void *ptr) {
    t_io_requests* io_request = (t_io_requests*) ptr;
    
    queue_destroy_and_destroy_elements(io_request->requests_queue, io_requests_queue_destroyer);
    
    free(io_request->requests_queue); // aca deberia hacer un free de nuevo?
    free(io_request);
}


void cpu_connections_destroyer(void *ptr) {
    t_cpu_connection* connection = (t_cpu_connection*) ptr;
    free(connection);
}