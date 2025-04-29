#include "lists.h"


static t_list* io_connections_list;
static t_queue* io_requests_queue;
static t_list* cpu_connections_list;

t_list* get_cpu_connections_list()
{
    return cpu_connections_list;
}

void initialize_global_lists() {

    io_connections_list = list_create();
    cpu_connections_list = list_create();
    io_requests_queue = queue_create();

    if(
        io_connections_list == NULL || 
        io_requests_queue == NULL ||
        cpu_connections_list == NULL
    ) {
        log_error(get_logger(), "Some of the lists/queues failed to create");
        exit(EXIT_FAILURE);
    }

    
}

void destroy_global_lists(){

    list_destroy_and_destroy_elements(io_connections_list, io_connections_destroyer);
    list_destroy_and_destroy_elements(cpu_connections_list, cpu_connections_destroyer);
    queue_destroy_and_destroy_elements(io_requests_queue, io_queue_destroyer);

}


void io_connections_destroyer(void *ptr) {
    t_io_connection* connection = (t_io_connection*) ptr;
    free(connection->device_name);
    free(connection);
}

void io_queue_destroyer(void *ptr) {
    t_io_queue* io_request = (t_io_queue*) ptr;
    free(io_request->device_name);
    free(io_request);
}

void cpu_connections_destroyer(void *ptr) {
    t_cpu_connection* connection = (t_cpu_connection*) ptr;
    free(connection);
}