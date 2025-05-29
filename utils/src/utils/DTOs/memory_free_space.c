#include "memory_free_space.h"

t_memory_free_space* read_memory_free_space(t_package* package) 
{
    package->buffer->offset = 0;
    t_memory_free_space* response = safe_malloc(sizeof(t_memory_free_space));
    
    response->free_space = buffer_read_uint32(package->buffer);
    
    package->buffer->offset = 0;
    return response;
}

int send_memory_free_space_request(int socket) 
{
    t_package* package = create_memory_free_space_request();
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_memory_free_space_request(void) 
{
    t_buffer* buffer = buffer_create(0);  // No data needed for request
    return package_create(GET_FREE_SPACE, buffer);
}

void destroy_memory_free_space(t_memory_free_space* response) 
{
    if (response) {
        free(response);
    }
}
