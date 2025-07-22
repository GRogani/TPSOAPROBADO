#include "confirmation_package.h"

t_package *create_confirmation_package(int32_t success)
{
    t_buffer *buffer = buffer_create(sizeof(int32_t) * 1);
    buffer_add_int32(buffer, success);
    return create_package(CONFIRMATION, buffer);
}

int send_confirmation_package(int socket, bool success)
{
    LOG_PACKAGE("Sending confirmation package: success: %s", success ? "true" : "false");
    t_package *package = create_confirmation_package((int32_t)success);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

bool read_confirmation_package(t_package *package)
{
    package->buffer->offset = 0;
    bool success = (bool)buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read confirmation package: success: %s", success ? "true" : "false");
    return success;
}