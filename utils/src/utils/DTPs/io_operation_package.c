#include "io_operation_package.h"

t_package* create_io_operation_package(uint32_t pid, uint32_t time) 
{
    uint32_t size = sizeof(uint32_t) * 2;
    t_buffer* buffer = buffer_create(size);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, time);
    return create_package(IO_OPERATION, buffer);
}

int send_io_operation_package(int socket, uint32_t pid, uint32_t time) 
{
    LOG_PACKAGE("Sending IO operation package: pid: %u, time: %u", pid, time);
    t_package* package ;
    package = create_io_operation_package(pid, time);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

io_operation_package_data* read_io_operation_package(t_package* package) 
{
    package->buffer->offset = 0;
    io_operation_package_data* io = safe_malloc(sizeof(io_operation_package_data));
    io->pid = buffer_read_uint32(package->buffer);
    io->sleep_time = buffer_read_uint32(package->buffer);
    package->buffer->offset = 0;
    LOG_PACKAGE("Read IO operation package: pid: %u, sleep_time: %u", io->pid, io->sleep_time);
    return io;
}