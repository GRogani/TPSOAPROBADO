#include "memory_read_request.h"

t_memory_read_request* create_memory_read_request(uint32_t physical_address, uint32_t size) {
    t_memory_read_request* request = safe_malloc(sizeof(t_memory_read_request));
    request->physical_address = physical_address;
    request->size = size;
    return request;
}

void destroy_memory_read_request(t_memory_read_request* request) {
    free(request);
}

void send_memory_read_request(int socket, t_memory_read_request* request) {
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, request->physical_address);
    buffer_add_uint32(buffer, request->size);
    t_package* package = package_create(READ_MEMORY, buffer);
    send_package(socket, package);
    package_destroy(package);
}

t_memory_read_request* read_memory_read_request(t_package* package) {
    t_memory_read_request* request = safe_malloc(sizeof(t_memory_read_request));
    package->buffer->offset = 0;
    request->physical_address = buffer_read_uint32(package->buffer);
    request->size = buffer_read_uint32(package->buffer);
    return request;
}
