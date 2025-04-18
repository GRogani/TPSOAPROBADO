#include <main.h>

int main(int argc, char* argv[]) {
    initialize_global_vars();

    signal(SIGINT, shutdown_hook);
    signal(SIGTERM, shutdown_hook);


    create_server_io();
    create_servers_cpu();


    while(1) pause(); // TODO: use joinable threads and remove this line. detachable threads could die and you will never notice
    return 0;
}

void create_server_io() {
    log_info(get_logger(), "Creating detachable thread for I/O server");
    pthread_t t1;
    int err = pthread_create(&t1, NULL, create_io_server, NULL);
    if(err) {
        log_error(get_logger(), "Failed to create detachable thread for I/O server");
        exit(EXIT_FAILURE);
    }
}

void create_servers_cpu() {
    log_info(get_logger(), "Creating detachable thread for CPUs servers");
    pthread_t t2;
    int err = pthread_create(&t2, NULL, create_cpu_servers, NULL);
    if(err) {
        log_error(get_logger(), "Failed to create detachable thread for CPU servers");
        exit(EXIT_FAILURE);
    }
}

void initialize_global_vars() {
    config = init_config("kernel.config");
    kernel_config = init_kernel_config(config);

    init_logger(
        "kernel.log",
        "[Main]",
        kernel_config.log_level
    );

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