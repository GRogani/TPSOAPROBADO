#include "read-memory.h"
#include "../../user_space/user_space_memory.h"

void read_memory_request_handler(int client_socket, t_package* package) {
    LOG_DEBUG("Processing read memory request");
    
    t_memory_read_request* request = read_memory_read_request(package);
    if (request == NULL) {
        LOG_ERROR("Failed to parse read memory request");
        return;
    }
    
    LOG_DEBUG("Reading %u bytes from physical address %u", request->size, request->physical_address);
    
    char* buffer = malloc(request->size);
    if (buffer == NULL) {
        LOG_ERROR("Failed to allocate memory for read operation buffer");
        destroy_memory_read_request(request);
        return;
    }
    
    read_from_user_space(request->physical_address, buffer, request->size);

    delay_memory_access();

    send_memory_read_response(client_socket, buffer, request->size);
    
    LOG_DEBUG("Read memory response sent successfully");
    
    free(buffer);
    destroy_memory_read_request(request);
}
