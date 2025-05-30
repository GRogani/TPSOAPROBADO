#include "cpu_dispatch.h"

t_cpu_dispatch* read_cpu_dispatch(t_package* package) 
{
    package->buffer->offset = 0;
    t_cpu_dispatch* dispatch = safe_malloc(sizeof(t_cpu_dispatch));
    
    dispatch->pid = buffer_read_uint32(package->buffer);
    dispatch->pc = buffer_read_uint32(package->buffer);
    
    package->buffer->offset = 0;
    return dispatch;
}

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

void destroy_cpu_dispatch(t_cpu_dispatch* dispatch) 
{
    if (dispatch) {
        free(dispatch);
    }
}
