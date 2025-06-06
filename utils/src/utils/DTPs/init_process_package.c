#include "init_process_package.h"

t_package* create_init_process_package(uint32_t pid, uint32_t size, char* pseudocode_path) 
{
    uint32_t path_len = pseudocode_path ? strlen(pseudocode_path) + 1 : 1;
    uint32_t buffer_size = sizeof(uint32_t) * 2 + path_len;
    
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, size);
    
    if (pseudocode_path) {
        buffer_add_string(buffer, strlen(pseudocode_path) + 1, pseudocode_path);
    } else {
        buffer_add_string(buffer, 0, "");
    }
    
    return create_package(CREATE_PROCESS, buffer);
}

int send_init_process_package(int socket, uint32_t pid, uint32_t size, char* pseudocode_path) 
{
    t_package* package = create_init_process_package(pid, size, pseudocode_path);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

init_process_package_data* read_init_process_package(t_package* package) 
{
    package->buffer->offset = 0;
    init_process_package_data* request = safe_malloc(sizeof(init_process_package_data));
    
    request->pid = buffer_read_uint32(package->buffer);
    request->size = buffer_read_uint32(package->buffer);
    
    uint32_t path_len;
    request->pseudocode_path = buffer_read_string(package->buffer, &path_len);
    
    package->buffer->offset = 0;
    return request;
}

void destroy_init_process_package(init_process_package_data* request) 
{
    if (request) {
        if (request->pseudocode_path) {
            free(request->pseudocode_path);
        }
        free(request);
    }
}
