#include "io_operation_request.h"

t_request_io* read_io_operation_request(t_package* package) {
    package->buffer->offset = 0;
    t_request_io* io = safe_malloc(sizeof(t_request_io));
    io->pid = buffer_read_uint32(package->buffer);
    io->sleep_time = buffer_read_uint32(package->buffer);
    package->buffer->offset = 0;
    return io;
}

int send_io_operation_request(int socket, uint32_t pid, uint32_t time) {
    t_package* package ;
    package = create_io_operation_request(pid, time);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_io_operation_request(uint32_t pid, uint32_t time) {
    uint32_t size = sizeof(uint32_t) * 2;
    t_buffer* buffer = buffer_create(size);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, time);
    return package_create(REQUEST_IO, buffer);
}