#include "fetch_package.h"

fetch_data read_fetch_package(t_package* package) 
{
    fetch_data fetch;
    package->buffer->offset = 0;   
    fetch.pid = buffer_read_uint32(package->buffer);
    fetch.pc = buffer_read_uint32(package->buffer);
    
    return fetch;
}

t_package* create_fetch_package(uint32_t pid, uint32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return create_package(FETCH, buffer);
}

void send_fetch_package(int socket, uint32_t pid, uint32_t pc) 
{
    t_package* package = create_fetch_package(pid, pc);
    send_package(socket, package);
    destroy_package(package);
}
