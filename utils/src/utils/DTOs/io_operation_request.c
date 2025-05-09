#include "dtos.h"

t_request_IO* read_IO_operation_request(t_package* package) {
    package->buffer->offset = 0;
    t_request_IO* io = safe_malloc(sizeof(t_request_IO));
    io->pid = buffer_read_uint32(package->buffer);
    io->sleep_time = buffer_read_uint32(package->buffer);
    package->buffer->offset = 0;
    return io;
}

int send_IO_operation_request(int socket, uint32_t pid, uint32_t time) {
    t_package* package ;
    package = create_IO_operation_request(pid, time);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_IO_operation_request(uint32_t pid, uint32_t time) {
    uint32_t size = sizeof(uint32_t) * 2;
    t_buffer* buffer = buffer_create(size);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, time);
    return package_create(IO, buffer);
}