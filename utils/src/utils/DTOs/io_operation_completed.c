#include "dtos.h"

char* read_IO_operation_completed(t_package* package){
    package->buffer->offset = 0;
    uint32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    return result;
}

int send_IO_operation_completed(int kernel_socket, char* yourName){
    t_package* package = safe_malloc(sizeof(t_package));
    package = create_IO_operation_completed(yourName);
    int bytes_sent = send_package(kernel_socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_IO_operation_completed(char* yourName){
    uint32_t size = strlen(yourName) + 1;
    t_buffer* buffer = buffer_create(size);
    buffer_add_string(buffer, size, yourName);;
    return package_create(IO, buffer);
}