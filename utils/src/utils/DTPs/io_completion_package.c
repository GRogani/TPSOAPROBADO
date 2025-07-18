#include "io_completion_package.h"

t_package* create_io_completion_package(char* device_name, uint32_t pid){
    uint32_t str_size = strlen(device_name) + 1;
    uint32_t size = str_size + sizeof(uint32_t);
    t_buffer* buffer = buffer_create(size);
    buffer_add_string(buffer, str_size, device_name);
    buffer_add_uint32(buffer, pid);
    return create_package(IO_COMPLETION, buffer);
}

int send_io_completion_package(int kernel_socket, char* device_name, uint32_t pid){
    t_package* package;
    package = create_io_completion_package(device_name, pid);
    int bytes_sent = send_package(kernel_socket, package);
    destroy_package(package);
    return bytes_sent;
}

io_completion_package_data* read_io_completion_package(t_package* package){
    package->buffer->offset = 0;
    uint32_t bytes_read;
    io_completion_package_data* result = malloc(sizeof(io_completion_package_data));
    result->device_name = buffer_read_string(package->buffer, &bytes_read);
    result->pid = buffer_read_uint32(package->buffer);
    package->buffer->offset = 0;
    return result;
}
