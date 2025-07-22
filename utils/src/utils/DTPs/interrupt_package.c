#include "interrupt_package.h"

t_package* create_interrupt_package (int32_t pid) 
{
    t_buffer* buffer = buffer_create(sizeof(int32_t));
    buffer_add_int32(buffer, pid);
    return create_package(INTERRUPT, buffer);
}

int send_interrupt_package (int socket, int32_t pid) 
{
    LOG_PACKAGE("Sending interrupt package: pid: %u", pid);
    t_package* package = create_interrupt_package(pid);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

int read_interrupt_package (t_package *package)
{
    package->buffer->offset = 0;
    int pid = buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read interrupt package: pid: %d", pid);
    return pid;
}