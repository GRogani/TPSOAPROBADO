#include "cpu_interrupt.h"

t_cpu_interrupt* read_cpu_interrupt(t_package* package) 
{
    package->buffer->offset = 0;
    t_cpu_interrupt* interrupt = safe_malloc(sizeof(t_cpu_interrupt));
    
    interrupt->pid = buffer_read_uint32(package->buffer);
    interrupt->pc = buffer_read_uint32(package->buffer);
    
    package->buffer->offset = 0;
    return interrupt;
}

t_package* create_cpu_interrupt_request(uint32_t pid) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, pid);
    return package_create(CPU_INTERRUPT, buffer);
}

int send_cpu_interrupt_request(int socket, uint32_t pid) 
{
    t_package* package = create_cpu_interrupt_request(pid);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_cpu_interrupt_response(uint32_t pid, uint32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return package_create(CPU_INTERRUPT, buffer);
}

int send_cpu_interrupt_response(int socket, uint32_t pid, uint32_t pc) 
{
    t_package* package = create_cpu_interrupt_response(pid, pc);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

void destroy_cpu_interrupt(t_cpu_interrupt* interrupt) 
{
    if (interrupt) {
        free(interrupt);
    }
}
