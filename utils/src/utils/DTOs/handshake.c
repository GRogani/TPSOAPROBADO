#include "dtos.h"

char* read_handshake(t_package* package)
{
    package->buffer->offset = 0;
    uint32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    return result;
}

t_package* create_handshake(char* yourName)
{

    uint32_t len = strlen(yourName) + 1;
    t_buffer* buffer = buffer_create(len);
    buffer_add_string(buffer, len, yourName);
    return package_create(HANDSHAKE, buffer);

}

int send_handshake(int socket, char* yourName)
{
    t_package* pkg = create_handshake(yourName);
    int bytes_sent = send_package(socket, pkg);
    package_destroy(pkg);
    return bytes_sent;
}

int send_IO_operation_request(int socket, int32_t pid, int32_t time) {
    t_package* package = safe_malloc(sizeof(t_package));
    package = create_IO_operation_request(pid, time);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_IO_operation_request(int32_t pid, int32_t time) {
    uint32_t size = sizeof(int32_t) * 2;
    t_buffer* buffer = buffer_create(size);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, time);
    return package_create(IO, buffer);
}

char* read_IO_operation_request(t_package* package){
    package->buffer->offset = 0;
    uint32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    return result;
}

int send_IO_operation_completed(int kernel_socket, char* yourName){
    t_package* package = safe_malloc(sizeof(t_package));
    package = create_IO_operation_completed(yourName);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_IO_operation_completed(char* yourName){
    uint32_t size = strlen(yourName) + 1;
    t_buffer* buffer = buffer_create(size);
    buffer_add_string(buffer, size, yourName);;
    return package_create(IO, buffer);
}