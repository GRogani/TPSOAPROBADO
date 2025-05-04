#include "dtos.h"

t_package* create_io_request(uint32_t pid, uint32_t sleep_time)
{
    uint32_t len = 2 * sizeof(uint32_t);
    t_buffer* buffer = buffer_create(len);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, sleep_time);
    return package_create(REQUEST_IO, buffer);

}

int send_io_request(int socket, uint32_t pid, uint32_t sleep_time)
{
    t_package* pkg = create_io_request(pid, sleep_time);
    int bytes_sent = send_package(socket, pkg);
    package_destroy(pkg);
    return bytes_sent;
}

