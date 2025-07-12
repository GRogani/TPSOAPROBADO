#include "fetch_package.h"

t_package* create_fetch_package(int32_t pid, int32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(int32_t) * 2);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return create_package(FETCH, buffer);
}

void send_fetch_package(int socket, int32_t pid, int32_t pc) 
{
    t_package* package = create_fetch_package(pid, pc);
    send_package(socket, package);
    destroy_package(package);
}


fetch_package_data read_fetch_package(t_package* package) 
{
    fetch_package_data fetch;
    package->buffer->offset = 0;   
    fetch.pid = buffer_read_uint32(package->buffer);
    fetch.pc = buffer_read_uint32(package->buffer);
    
    return fetch;
}