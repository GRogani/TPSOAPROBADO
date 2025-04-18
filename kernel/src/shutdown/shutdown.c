#include "shutdown.h"
#include <stdio.h>
#include <stdlib.h>

void shutdown_hook() {
    log_info(get_logger(), "Shutting down kernel...");

    list_destroy_and_destroy_elements(io_connections_list, io_connections_destroyer);
    list_destroy_and_destroy_elements(cpu_connections_list, cpu_connections_destroyer);
    queue_destroy_and_destroy_elements(io_requests_queue, io_queue_destroyer);
    
    config_destroy(config);
    destroy_logger(get_logger());

    LOG_DEBUG("Kernel died gracefuly");
    
    exit(EXIT_SUCCESS);
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