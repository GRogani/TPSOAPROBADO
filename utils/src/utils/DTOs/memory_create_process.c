#include "memory_create_process.h"

t_memory_create_process* read_memory_create_process(t_package* package) 
{
    package->buffer->offset = 0;
    t_memory_create_process* request = safe_malloc(sizeof(t_memory_create_process));
    
    request->pid = buffer_read_uint32(package->buffer);
    request->size = buffer_read_uint32(package->buffer);
    
    uint32_t path_len;
    request->pseudocode_path = buffer_read_string(package->buffer, &path_len);
    
    package->buffer->offset = 0;
    return request;
}

int send_memory_create_process_request(int socket, uint32_t pid, uint32_t size, const char* pseudocode_path) 
{
    t_package* package = create_memory_create_process_request(pid, size, pseudocode_path);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

t_package* create_memory_create_process_request(uint32_t pid, uint32_t size, const char* pseudocode_path) 
{
    uint32_t path_len = pseudocode_path ? strlen(pseudocode_path) + 1 : 1;
    uint32_t buffer_size = sizeof(uint32_t) * 2 + path_len;
    
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, size);
    
    if (pseudocode_path) {
        buffer_add_string(buffer, strlen(pseudocode_path), pseudocode_path);
    } else {
        buffer_add_string(buffer, 0, "");
    }
    
    return package_create(CREATE_PROCESS, buffer);
}

void destroy_memory_create_process(t_memory_create_process* request) 
{
    if (request) {
        if (request->pseudocode_path) {
            free(request->pseudocode_path);
        }
        free(request);
    }
}
