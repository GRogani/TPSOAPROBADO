#include "dtos.h"

char* read_io_handshake(t_package* package)
{
    package->buffer->offset = 0;
    uint32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    return result;
}

t_package* create_io_handshake(char* yourName)
{

    uint32_t len = strlen(yourName) + 1;
    t_buffer* buffer = buffer_create(len);
    buffer_add_string(buffer, len, yourName);
    return package_create(HANDSHAKE, buffer);

}

int send_io_handshake(int socket, char *yourName)
{
    t_package* pkg = create_io_handshake(yourName);
    int bytes_sent = send_package(socket, pkg);
    package_destroy(pkg);
    return bytes_sent;
}

