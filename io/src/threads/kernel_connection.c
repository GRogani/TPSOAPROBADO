#include "kernel_connection.h"

void* create_kernel_connection(void* args) {
    void** argv = (void**) args;

    int* socket_ptr = (int*) argv[0];
    char* puerto = (char*) argv[1];
    char* ip = (char*) argv[2];

    do {
        *socket_ptr = create_connection(puerto, ip);

        if (*socket_ptr != -1)
            log_info(get_logger(), "Connection to Kernel established: %s::%s", ip, puerto);
        else {
            log_error(get_logger(), "Connection to Kernel failed: %s::%s", ip, puerto);
            log_info(get_logger(), "Retrying connection in 3 seconds");
            sleep(3);
        }
    } while (*socket_ptr == -1);

    return NULL;
}
