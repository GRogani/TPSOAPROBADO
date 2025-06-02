#include "memory_suspend_process.h"

// REQUEST de KERNEL
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
    return package_create(SWAP, buffer);
}

uint32_t read_memory_suspend_process(t_package* package) { // Lectura para Memoria
    package->buffer->offset = 0;
    uint32_t pid = buffer_read_uint32(package->buffer);
    return pid;
}

// RESPONSE de MEMORIA
int send_memory_suspend_process_response(int socket_fd, uint32_t success) {
    t_package* package = create_memory_suspend_process_response(success);
    int bytes_sent = send_package(socket_fd, package);
    if (bytes_sent < 0) LOG_ERROR("DTO: Error al enviar paquete SUSPEND_PROCESS_RESPONSE a Kernel");
    package_destroy(package);
}

t_package* create_memory_suspend_process_response(uint32_t success) {
    uint32_t buffer_size = sizeof(uint32_t);
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_uint32(buffer, success);
    return package_create(SWAP, buffer);
}

bool read_memory_suspend_process_response(t_package* package) { // Lectura para el KERNEL
    package->buffer->offset = 0;
    uint32_t success = buffer_read_uint32(package->buffer);
    return success == 1;
}