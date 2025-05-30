#include "cpu_syscall_request.h"

t_cpu_syscall_request* read_cpu_syscall_request(t_package* package) 
{
    package->buffer->offset = 0;
    t_cpu_syscall_request* response = safe_malloc(sizeof(t_cpu_syscall_request));
    
    response->syscall_type = buffer_read_uint32(package->buffer);
    response->pid = buffer_read_uint32(package->buffer);
    response->pc = buffer_read_uint32(package->buffer);
    
    package->buffer->offset = 0;
    return response;
}

t_package* create_cpu_syscall_request(t_syscall_request_type syscall_type, uint32_t pid, uint32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 3);
    buffer_add_uint32(buffer, (uint32_t)syscall_type);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return package_create(CPU_SYSCALL, buffer);
}

int send_cpu_syscall_request(int socket, t_syscall_request_type syscall_type, uint32_t pid, uint32_t pc) 
{
    t_package* package = create_cpu_syscall_request(syscall_type, pid, pc);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

void destroy_cpu_syscall_response(t_cpu_syscall_request* response) 
{
    if (response) {
        free(response);
    }
}
