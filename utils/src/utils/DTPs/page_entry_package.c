#include "page_entry_package.h"

t_package* create_page_entry_request_package(int32_t pid, int32_t table_id, int32_t entry_index)
{
    t_buffer* buffer = buffer_create(sizeof(int32_t) * 3);
    buffer_add_int32(buffer, pid);
    buffer_add_int32(buffer, table_id);
    buffer_add_int32(buffer, entry_index);
    return create_package(GET_PAGE_ENTRY, buffer);
}

void send_page_entry_request_package(int socket, int32_t pid, int32_t table_id, int32_t entry_index)
{
    LOG_PACKAGE("Sending page entry request package: pid: %u, table_id: %u, entry_index: %u", pid, table_id, entry_index);
    t_package* package = create_page_entry_request_package(pid, table_id, entry_index);
    send_package(socket, package);
    destroy_package(package);
}

page_entry_request_data read_page_entry_request_package(t_package* package)
{
    page_entry_request_data data;
    package->buffer->offset = 0;
    data.pid = buffer_read_int32(package->buffer);
    data.table_id = buffer_read_int32(package->buffer);
    data.entry_index = buffer_read_int32(package->buffer);
    LOG_PACKAGE("Read page entry request package: pid: %u, table_id: %u, entry_index: %u", data.pid, data.table_id, data.entry_index);
    return data;
}

t_package* create_page_entry_response_package(int32_t value, int32_t is_last_level)
{
    t_buffer* buffer = buffer_create(sizeof(int32_t) * 2);
    buffer_add_int32(buffer, value);
    buffer_add_int32(buffer, is_last_level);
    return create_package(GET_PAGE_ENTRY, buffer);
}

void send_page_entry_response_package(int socket, int32_t value, int32_t is_last_level)
{
    t_package* package = create_page_entry_response_package(value, is_last_level);
    send_package(socket, package);
    destroy_package(package);
}

page_entry_response_data read_page_entry_response_package(t_package* package)
{
    page_entry_response_data data;
    package->buffer->offset = 0;
    data.value = buffer_read_int32(package->buffer);
    data.is_last_level = buffer_read_int32(package->buffer) == 0;
    
    return data;
}
