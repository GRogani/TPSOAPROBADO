#include "mmu_request_page_write_to_memory.h"

// read from cpu/mmu
t_mmu_page_write_request* read_mmu_page_write_request(t_package* package) 
{
    package->buffer->offset = 0;
    t_mmu_page_write_request* request = safe_malloc(sizeof(t_mmu_page_write_request));
    
    request->frame_number = buffer_read_uint32(package->buffer);
    request->content_size = buffer_read_uint32(package->buffer);
    
    if (request->content_size > 0) {
        request->content = safe_malloc(request->content_size);
        buffer_read(package->buffer, request->content, request->content_size);
    } else {
        request->content = NULL;
    }

    return request;
}

// sent by cpu/mmu to memory
t_package* create_mmu_page_write_request(uint32_t frame_number, void* content, uint32_t content_size) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2 + content_size);
    buffer_add_uint32(buffer, frame_number);
    buffer_add_uint32(buffer, content_size);
    
    if (content_size > 0 && content != NULL) {
        buffer_add(buffer, content, content_size);
    }
    
    return package_create(MMU_PAGE_WRITE_REQUEST, buffer);
}

int send_mmu_page_write_request(int socket, uint32_t frame_number, void* content, uint32_t content_size) 
{
    t_package* package = create_mmu_page_write_request(frame_number, content, content_size);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}

void destroy_mmu_page_write_request(t_mmu_page_write_request* request) 
{
    if (request) {
        if (request->content) {
            free(request->content);
        }
        free(request);
    }
}