#include "dtos.h"

/** 
 * @brief Returns pid 
 */
int read_io_completion(t_package* package)
{
    package->buffer->offset = 0;
    char* result = buffer_read_uint32(package->buffer);
    package->buffer->offset = 0;
    return result;
}

t_package* create_io_completion(int pid)
{

    uint32_t len = sizeof(uint32_t);
    t_buffer* buffer = buffer_create(len);
    buffer_add_uint32(buffer, pid);
    return package_create(IO_COMPLETION, buffer);

}

int send_io_completion(int socket, int pid)
{
    t_package* pkg = create_io_handshake(pid);
    int bytes_sent = send_package(socket, pkg);
    package_destroy(pkg);
    return bytes_sent;
}

