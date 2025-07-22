#include "new_io_package.h"

t_package* create_new_io_package(char* device_name){
    t_buffer* buffer = buffer_create(0);
    buffer_add_string(buffer, strlen(device_name) + 1, device_name);
    return create_package(NEW_IO, buffer);
};

int send_new_io_package(int kernel_socket, char* device_name)
{
    LOG_PACKAGE("Sending new IO package: device_name: %s", device_name);
    t_package* package;
    package = create_new_io_package(device_name);
    int bytes_sent = send_package(kernel_socket, package);
    LOG_INFO("New device '%s' registered in Kernel", device_name);
    destroy_package(package);
    return bytes_sent;
};

char* read_new_io_package(t_package* package)
{
    package->buffer->offset = 0;
    int32_t bytes_read;
    char* result = buffer_read_string(package->buffer, &bytes_read);
    package->buffer->offset = 0;
    LOG_PACKAGE("Read new IO package: device_name: %s", result);
    return result;
};