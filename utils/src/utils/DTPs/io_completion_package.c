#include "io_completion_package.h"

t_package* create_io_completion_package(char* device_name, int32_t pid){
    int32_t str_size = strlen(device_name) + 1;
    int32_t size = str_size + sizeof(int32_t);
    t_buffer* buffer = buffer_create(size);
    buffer_add_string(buffer, str_size, device_name);
    buffer_add_int32(buffer, pid);
    return create_package(IO_COMPLETION, buffer);
}

int send_io_completion_package(int kernel_socket, char* device_name, int32_t pid){
    LOG_PACKAGE("Sending IO completion package: device_name: %s, pid: %u", device_name, pid);
    t_package* package;
    package = create_io_completion_package(device_name, pid);
    int bytes_sent = send_package(kernel_socket, package);
    destroy_package(package);
    return bytes_sent;
}

io_completion_package_data* read_io_completion_package(t_package* package){
    package->buffer->offset = 0;
    int32_t bytes_read;
    io_completion_package_data* result = malloc(sizeof(io_completion_package_data));
    result->device_name = buffer_read_string(package->buffer, &bytes_read);
    result->pid = buffer_read_int32(package->buffer);
    package->buffer->offset = 0;
    LOG_PACKAGE("Read IO completion package: device_name: %s, pid: %u", result->device_name, result->pid);
    return result;
}
