#include "write-memory.h"
#include "../../user_space/user_space_memory.h"

void write_memory_request_handler(int client_socket, t_package* package) {
    LOG_DEBUG("Processing write memory request");
    
    t_memory_write_request* request = read_memory_write_request(package);
    if (request == NULL) {
        LOG_ERROR("Failed to parse write memory request");
        send_confirmation_package(client_socket, false);
        return;
    }
    
    LOG_DEBUG("Writing %u bytes to physical address %u", request->size, request->physical_address);
    
    write_to_user_space(request->physical_address, request->data, request->size);

    delay_memory_access();

    send_confirmation_package(client_socket, true);

    LOG_DEBUG("Write memory confirmation sent successfully");
    
    destroy_memory_write_request(request);
}
