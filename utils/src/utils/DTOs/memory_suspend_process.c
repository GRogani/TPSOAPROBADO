#include "memory_suspend_process.h"

int send_memory_suspend_process(int socket, uint32_t pid) {
    t_package* package = create_memory_suspend_process(pid);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package *create_memory_suspend_process(uint32_t pid){
    uint32_t buffer_size = sizeof(uint32_t);
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_uint32(buffer, pid);
    return package_create(SUSP_BLOCKED, buffer);
}