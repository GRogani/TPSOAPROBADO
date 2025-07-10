#include "page_entry_package.h"

t_package* create_page_entry_request_package(uint32_t pid, uint32_t table_ptr, uint32_t entry_index)
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 3);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, table_ptr);
    buffer_add_uint32(buffer, entry_index);
    return create_package(GET_PAGE_ENTRY, buffer);
}

void send_page_entry_request_package(int socket, uint32_t pid, uint32_t table_ptr, uint32_t entry_index)
{
    t_package* package = create_page_entry_request_package(pid, table_ptr, entry_index);
    send_package(socket, package);
    destroy_package(package);
}

page_entry_request_data read_page_entry_request_package(t_package* package)
{
    page_entry_request_data data;
    package->buffer->offset = 0;
    data.pid = buffer_read_uint32(package->buffer);
    data.table_ptr = buffer_read_uint32(package->buffer);
    data.entry_index = buffer_read_uint32(package->buffer);
    
    return data;
}

t_package* create_page_entry_response_package(uint32_t value, uint32_t is_last_level)
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, value);
    buffer_add_uint32(buffer, is_last_level);
    return create_package(GET_PAGE_ENTRY, buffer);
}

void send_page_entry_response_package(int socket, uint32_t value, uint32_t is_last_level)
{
    t_package* package = create_page_entry_response_package(value, is_last_level);
    send_package(socket, package);
    destroy_package(package);
}

page_entry_response_data read_page_entry_response_package(t_package* package)
{
    page_entry_response_data data;
    package->buffer->offset = 0;
    data.value = buffer_read_uint32(package->buffer);
    data.is_last_level = buffer_read_uint32(package->buffer) == 0;
    
    return data;
}
