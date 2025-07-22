#include "dispatch_package.h"

t_package* create_dispatch_package(int32_t pid, int32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(int32_t) * 2);
    buffer_add_int32(buffer, pid);
    buffer_add_int32(buffer, pc);
    return create_package(DISPATCH, buffer);
}

int send_dispatch_package(int socket, int32_t pid, int32_t pc) 
{
    LOG_PACKAGE("Sending dispatch package: pid: %u, pc: %u", pid, pc);
    t_package* package = create_dispatch_package(pid, pc);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

dispatch_package_data read_dispatch_package(t_package* package) 
{
    dispatch_package_data package_data;
    package->buffer->offset = 0;   
    package_data.pid = buffer_read_int32(package->buffer);
    package_data.pc = buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read dispatch package: pid: %u, pc: %u", package_data.pid, package_data.pc);
    return package_data;
}