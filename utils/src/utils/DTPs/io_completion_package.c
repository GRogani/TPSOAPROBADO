#include "io_completion_package.h"

t_package* create_io_completion_package(char* device_name){
    uint32_t size = strlen(device_name) + 1;
    t_buffer* buffer = buffer_create(size);
    buffer_add_string(buffer, size, device_name);
    return create_package(IO_COMPLETION, buffer);
}

int send_io_completion_package(int kernel_socket, char* device_name){
    t_package* package;
    package = create_io_completion_package(device_name);
    int bytes_sent = send_package(kernel_socket, package);
    destroy_package(package);
    return bytes_sent;
}


char* read_io_completion_package(t_package* package){
    package->buffer->offset = 0;
    uint32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    return result;
}
