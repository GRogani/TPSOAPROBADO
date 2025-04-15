#include <signal.h>
#include <io/handle-io-server.h>
#include <cpu/handle-cpu-servers.h>

#include<main.h>

t_log* logger;
t_list* io_connections_list;
t_queue* io_requests_queue;
t_list* cpu_connections_list;
t_kernel_config* kernel_config;

void shutdown_hook(t_kernel_config* kernel_config, t_config* config, t_log* logger) {
    list_destroy_and_destroy_elements(io_connections_list, io_connections_destroyer);
    list_destroy_and_destroy_elements(cpu_connections_list, cpu_connections_destroyer);
    queue_destroy_and_destroy_elements(io_requests_queue, io_queue_destroyer);

    destroy_kernel_config(kernel_config);
    
    config_destroy(config);
    log_destroy(logger);

    printf("All memory cleared successfully");
}

int main(int argc, char* argv[]) {
    t_config* config = init_config_and_validate(kernel_config);

    initialize_global_vars();

    signal(SIGINT, shutdown_hook); // Ctrl+C
    signal(SIGTERM, shutdown_hook); // kill -15 PID


    create_server_io();
    create_servers_cpu();
    create_root_process();


    while(1) pause();
}

void create_server_io() {
    log_info(logger, "Creating detachable thread for I/O server");

    err = pthread_create(NULL, NULL, create_io_server, NULL);
    if(err) {
        log_error(logger, "Failed to create detachable thread for I/O server");
        exit(EXIT_FAILURE);
    }
}

void create_servers_cpu() {
    log_info(logger, "Creating detachable thread for CPUs servers");

    err = pthread_create(NULL, NULL, create_cpu_servers, NULL);
    if(err) {
        log_error(logger, "Failed to create detachable thread for CPU servers");
        exit(EXIT_FAILURE);
    }
}

void create_root_process() {
    // crear thread detachable
    // crear semaforo para esperar el init de la cpu.
}

void io_connections_destroyer(t_io_connection *connection) {
    free(connection->device_name);
    free(connection);
}

void io_queue_destroyer(t_io_queue *io_request) {
    free(io_request->device_name);
    free(io_request);
}

void cpu_connections_destroyer(t_cpu_connection *connection) {
    free(connection);
}

void initialize_global_vars() {
    logger = init_logger(
        "kernel.log",
        "[Main]",
        kernel_config->log_level
    );

    if(logger == NULL) {
        printf("Logger failed to initialize.");
        exit(EXIT_FAILURE);
    }

    kernel_config = malloc(sizeof(t_kernel_config));

    if (kernel_config == NULL) {
        log_error(logger, "Failed to allocate memory for kernel config.");
        exit(EXIT_FAILURE);
    }

    io_connections_list = list_create();
    io_requests_queue = queue_create();
    cpu_connections_list = list_create();

    if(
        io_connections_list == NULL || 
        io_requests_queue == NULL ||
        cpu_connections_list == NULL
    ) {
        log_error(logger, "Some of the lists/queues failed to create");
        exit(EXIT_FAILURE);
    }

    
}