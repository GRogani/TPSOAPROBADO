#include "cpu_context_package.h"

t_package *create_cpu_context_package(int32_t pid, int32_t pc, int32_t interrupted_same_pid)
{

    t_buffer* buffer = buffer_create(sizeof(int32_t) * 2);
    buffer_add_int32(buffer, pid);
    buffer_add_int32(buffer, pc);
    buffer_add_int32(buffer, interrupted_same_pid);
    return create_package(CPU_CONTEXT, buffer);
}

int send_cpu_context_package(int socket, int32_t pid, int32_t pc, int32_t interrupted_same_pid) 
{
    LOG_PACKAGE("Sending CPU context package: pid: %u, pc: %u, interrupted_same_pid: %d", pid, pc, interrupted_same_pid);
    t_package* package = create_cpu_context_package(pid, pc, interrupted_same_pid);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

cpu_context_package_data read_cpu_context_package(t_package *package)
{
    cpu_context_package_data cpu_context;
    package->buffer->offset = 0;
    cpu_context.pid = buffer_read_int32(package->buffer);
    cpu_context.pc = buffer_read_int32(package->buffer);
    cpu_context.interrupted_same_pid = buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read CPU context package: pid: %u, pc: %u, interrupted_same_pid: %d", cpu_context.pid, cpu_context.pc, cpu_context.interrupted_same_pid);
    return cpu_context;
}