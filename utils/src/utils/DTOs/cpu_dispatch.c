#include "cpu_dispatch.h"

// read from cpu
t_cpu_dispatch read_cpu_dispatch_request(t_package* package) 
{
    t_cpu_dispatch dispatch;
    package->buffer->offset = 0;   
    dispatch.pid = buffer_read_uint32(package->buffer);
    dispatch.pc = buffer_read_uint32(package->buffer);

    return dispatch;
}

// sent by kernel to cpu
t_package* create_cpu_dispatch_request(uint32_t pid, uint32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return package_create(PID_PC_PACKAGE, buffer);
}

int send_cpu_dispatch_request(int socket, uint32_t pid, uint32_t pc) 
{
    t_package* package = create_cpu_dispatch_request(pid, pc);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}
