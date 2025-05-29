#include "cpu_syscall.h"

t_cpu_syscall* read_cpu_syscall(t_package* package) 
{
    package->buffer->offset = 0;
    t_cpu_syscall* syscall = safe_malloc(sizeof(t_cpu_syscall));
    
    syscall->syscall_type = buffer_read_uint32(package->buffer);
    syscall->pid = buffer_read_uint32(package->buffer);
    
    switch (syscall->syscall_type) {
        case SYSCALL_INIT_PROC: {
            syscall->params.init_proc.memory_space = buffer_read_uint32(package->buffer);
            uint32_t filename_len;
            syscall->params.init_proc.pseudocode_file = buffer_read_string(package->buffer, &filename_len);
            break;
        }
        case SYSCALL_IO: {
            uint32_t device_len;
            syscall->params.io.device_name = buffer_read_string(package->buffer, &device_len);
            syscall->params.io.sleep_time = buffer_read_uint32(package->buffer);
            break;
        }
        case SYSCALL_DUMP_PROCESS:
        case SYSCALL_EXIT:
            // No additional parameters
            break;
    }
    
    package->buffer->offset = 0;
    return syscall;
}

t_package* create_cpu_syscall_response(uint32_t status) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, status);
    return package_create(CPU_SYSCALL_RESPONSE, buffer);
}

int send_cpu_syscall_response(int socket, uint32_t status) 
{
    t_package* package = create_cpu_syscall_response(status);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

void destroy_cpu_syscall(t_cpu_syscall* syscall) 
{
    if (syscall) {
        switch (syscall->syscall_type) {
            case SYSCALL_INIT_PROC:
                if (syscall->params.init_proc.pseudocode_file) {
                    free(syscall->params.init_proc.pseudocode_file);
                }
                break;
            case SYSCALL_IO:
                if (syscall->params.io.device_name) {
                    free(syscall->params.io.device_name);
                }
                break;
            default:
                break;
        }
        free(syscall);
    }
}
