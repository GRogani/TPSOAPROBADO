#include "fetch_package.h"

t_package* create_fetch_package(int32_t pid, int32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(int32_t) * 2);
    buffer_add_int32(buffer, pid);
    buffer_add_int32(buffer, pc);
    return create_package(FETCH, buffer);
}

void send_fetch_package(int socket, int32_t pid, int32_t pc) 
{
    LOG_PACKAGE("Sending fetch package: pid: %d, pc: %d", pid, pc);
    t_package* package = create_fetch_package(pid, pc);
    send_package(socket, package);
    destroy_package(package);
}


fetch_package_data read_fetch_package(t_package* package) 
{
    fetch_package_data fetch;
    package->buffer->offset = 0;   
    fetch.pid = buffer_read_int32(package->buffer);
    fetch.pc = buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read fetch package: pid: %d, pc: %d", fetch.pid, fetch.pc);
    return fetch;
}