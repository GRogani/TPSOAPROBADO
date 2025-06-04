#include "dispatch_package.h"

dispatch_package_data read_dispatch_package(t_package* package) 
{
    dispatch_package_data package_data;
    package->buffer->offset = 0;   
    package_data.pid = buffer_read_uint32(package->buffer);
    package_data.pc = buffer_read_uint32(package->buffer);

    return package_data;
}

t_package* create_dispatch_package(uint32_t pid, uint32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return create_package(DISPATCH, buffer);
}

int send_dispatch_package(int socket, uint32_t pid, uint32_t pc) 
{
    t_package* package = create_dispatch_package(pid, pc);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}
