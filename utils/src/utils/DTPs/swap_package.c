#include "swap_package.h"

// Wrappers
int send_swap_package(int socket, uint32_t pid) {
    return send_swap_operation_package(socket, pid, SWAP);
}
int send_swap_in_package(int socket, uint32_t pid) {
    return send_swap_operation_package(socket, pid, UNSUSPEND_PROCESS);
}

int send_swap_operation_package(int socket, uint32_t pid, OPCODE operation) {
    t_package* package = create_swap_operation_package(pid, operation);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

t_package *create_swap_operation_package(uint32_t pid, OPCODE operation) {
    uint32_t buffer_size = sizeof(uint32_t);
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_uint32(buffer, pid);
    return create_package(operation, buffer);
}

uint32_t read_swap_operation_package(t_package* package) {
    package->buffer->offset = 0;
    uint32_t pid = buffer_read_uint32(package->buffer);
    return pid;
}