#include "swap_package.h"

// Wrappers
int send_swap_package(int socket, int32_t pid) {
    return send_swap_operation_package(socket, pid, SWAP);
}
int send_swap_in_package(int socket, int32_t pid) {
    return send_swap_operation_package(socket, pid, UNSUSPEND_PROCESS);
}

int send_swap_operation_package(int socket, int32_t pid, OPCODE operation) {
    LOG_PACKAGE("Sending swap operation package: pid: %u, operation: %d", pid, operation);
    t_package* package = create_swap_operation_package(pid, operation);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

t_package *create_swap_operation_package(int32_t pid, OPCODE operation) {
    int32_t buffer_size = sizeof(int32_t);
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_int32(buffer, pid);
    return create_package(operation, buffer);
}

int32_t read_swap_package(t_package* package) {
    package->buffer->offset = 0;
    int32_t pid = buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read swap package: pid: %u", pid);
    return pid;
}