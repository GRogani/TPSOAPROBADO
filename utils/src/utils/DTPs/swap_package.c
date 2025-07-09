#include "swap_package.h"

int send_swap_package(int socket, uint32_t pid) {
    t_package* package = create_swap_package(pid);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

t_package *create_swap_package(uint32_t pid){
    uint32_t buffer_size = sizeof(uint32_t);
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_uint32(buffer, pid);
    return create_package(SWAP, buffer);
}

uint32_t read_swap_package(t_package* package) { // Lectura para Memoria
    package->buffer->offset = 0;
    uint32_t pid = buffer_read_uint32(package->buffer);
    return pid;
}