#include "dump_memory_package.h"

t_package *create_dump_memory_package(uint32_t pid) 
{
    t_buffer *buffer = buffer_create(sizeof(uint32_t));
    
    buffer_add_uint32(buffer, pid);

    return create_package(C_DUMP_MEMORY, buffer);
}

int send_dump_memory_package(int socket, uint32_t pid)
{
    LOG_PACKAGE("Sending dump memory package: pid: %u", pid);
    t_package *package = create_dump_memory_package(pid);

    int bytes_sent = send_package(socket, package);

    destroy_package(package);

    return bytes_sent;
}

uint32_t read_dump_memory_package(t_package *package)
{
    package->buffer->offset = 0;
    uint32_t pid = buffer_read_uint32(package->buffer);
    LOG_PACKAGE("Read dump memory package: pid: %u", pid);
    return pid;
}