#include "syscall_package.h"

t_package *create_syscall_package(syscall_package_data *syscall)
{
    t_buffer* buffer = buffer_create(sizeof(int32_t) * 3);
    buffer_add_int32(buffer, (int32_t)syscall->syscall_type);
    buffer_add_int32(buffer, syscall->pid);
    buffer_add_int32(buffer, syscall->pc);

    switch (syscall->syscall_type)
    {
    case SYSCALL_INIT_PROC:
    {
        buffer_add_int32(buffer, syscall->params.init_proc.memory_space);
        buffer_add_string(buffer, strlen(syscall->params.init_proc.pseudocode_file) + 1, syscall->params.init_proc.pseudocode_file);
        break;
    }
    case SYSCALL_IO:
    {
        buffer_add_string(buffer, strlen(syscall->params.io.device_name) + 1, syscall->params.io.device_name);
        buffer_add_int32(buffer, syscall->params.io.sleep_time);
        break;
    }
    case SYSCALL_DUMP_MEMORY:
    case SYSCALL_EXIT:
        // No additional parameters
        break;
    }

    return create_package(SYSCALL, buffer);
}

// used by CPU
int send_syscall_package(int socket, syscall_package_data *syscall) 
{
    LOG_PACKAGE("Sending syscall package: syscall_type: %d, pid: %u, pc: %u", syscall->syscall_type, syscall->pid, syscall->pc);
    t_package *package = create_syscall_package(syscall);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

syscall_package_data *read_syscall_package(t_package *package)
{
    package->buffer->offset = 0;
    syscall_package_data *syscall = safe_malloc(sizeof(syscall_package_data));

    syscall->syscall_type = buffer_read_int32(package->buffer);
    syscall->pid = buffer_read_int32(package->buffer);
    syscall->pc = buffer_read_int32(package->buffer);

    switch (syscall->syscall_type)
    {
    case SYSCALL_INIT_PROC:
    {
        syscall->params.init_proc.memory_space = buffer_read_int32(package->buffer);
        int32_t filename_len;
        syscall->params.init_proc.pseudocode_file = buffer_read_string(package->buffer, &filename_len);
        break;
    }
    case SYSCALL_IO:
    {
        int32_t device_len;
        syscall->params.io.device_name = buffer_read_string(package->buffer, &device_len);
        syscall->params.io.sleep_time = buffer_read_int32(package->buffer);
        break;
    }
    case SYSCALL_DUMP_MEMORY:
    case SYSCALL_EXIT:
        // No additional parameters
        break;
    }

    package->buffer->offset = 0;
    LOG_PACKAGE("Read syscall package: syscall_type: %d, pid: %u, pc: %u", syscall->syscall_type, syscall->pid, syscall->pc);
    return syscall;
}

void destroy_syscall_package(syscall_package_data *syscall)
{
    if (syscall)
    {
        switch (syscall->syscall_type)
        {
        case SYSCALL_INIT_PROC:
            if (syscall->params.init_proc.pseudocode_file)
            {
                free(syscall->params.init_proc.pseudocode_file);
            }
            break;
        case SYSCALL_IO:
            if (syscall->params.io.device_name)
            {
                free(syscall->params.io.device_name);
            }
            break;
        default:
            break;
        }
        free(syscall);
    }
}
