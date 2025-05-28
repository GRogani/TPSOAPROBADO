#include "io_operation_completed.h"

char* read_io_operation_completed(t_package* package){
    package->buffer->offset = 0;
    uint32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    return result;
}

int send_io_operation_completed(int kernel_socket, char* device_name){
    t_package* package = safe_malloc(sizeof(t_package));
    package = create_io_operation_completed(device_name);
    int bytes_sent = send_package(kernel_socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_io_operation_completed(char* device_name){
    uint32_t size = strlen(device_name) + 1;
    t_buffer* buffer = buffer_create(size);
    buffer_add_string(buffer, size, device_name);
    return package_create(IO_COMPLETION, buffer);
}