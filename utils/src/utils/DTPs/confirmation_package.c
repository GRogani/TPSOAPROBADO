#include "confirmation_package.h"

t_package *create_confirmation_package(uint32_t success)
{
    t_buffer *buffer = buffer_create(sizeof(uint32_t) * 1);
    buffer_add_uint32(buffer, success); // 0 = success, 1 = failure
    return create_package(CONFIRMATION, buffer);
}

int send_confirmation_package(int socket, uint32_t success)
{
    t_package *package = create_confirmation_package(success);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

bool read_confirmation_package(t_package *package)
{
    package->buffer->offset = 0;
    int success = buffer_read_uint32(package->buffer);
    return success == 0; // Return 0 for success, otherwise failure
}