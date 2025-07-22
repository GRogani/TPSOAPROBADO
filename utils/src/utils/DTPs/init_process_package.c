#include "init_process_package.h"

t_package* create_init_process_package(int32_t pid, int32_t size, char* pseudocode_path) 
{
    int32_t path_len = pseudocode_path ? strlen(pseudocode_path) + 1 : 1;
    int32_t buffer_size = sizeof(int32_t) * 2 + path_len;
    
    t_buffer* buffer = buffer_create(buffer_size);
    buffer_add_int32(buffer, pid);
    buffer_add_int32(buffer, size);
    
    if (pseudocode_path) {
        buffer_add_string(buffer, strlen(pseudocode_path) + 1, pseudocode_path);
    } else {
        buffer_add_string(buffer, 0, "");
    }
    
    return create_package(INIT_PROCESS, buffer);
}

int send_init_process_package(int socket, int32_t pid, int32_t size, char* pseudocode_path) 
{
    LOG_PACKAGE("Sending init process package: pid: %u, size: %u, pseudocode_path: %s", pid, size, pseudocode_path ? pseudocode_path : "NULL");
    t_package* package = create_init_process_package(pid, size, pseudocode_path);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

init_process_package_data* read_init_process_package(t_package* package) 
{
    package->buffer->offset = 0;
    init_process_package_data* request = safe_malloc(sizeof(init_process_package_data));
    
    request->pid = buffer_read_int32(package->buffer);
    request->size = buffer_read_int32(package->buffer);
    
    int32_t path_len;
    request->pseudocode_path = buffer_read_string(package->buffer, &path_len);
    
    package->buffer->offset = 0;
    LOG_PACKAGE("Read init process package: pid: %u, size: %u, pseudocode_path: %s", request->pid, request->size, request->pseudocode_path ? request->pseudocode_path : "NULL");
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
