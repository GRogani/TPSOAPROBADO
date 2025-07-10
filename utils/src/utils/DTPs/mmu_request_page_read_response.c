#include "mmu_request_page_read_response.h"

// read by cpu/mmu
t_mmu_page_read_response *read_mmu_page_read_response(t_package *package)
{
  t_mmu_page_read_response *response = safe_malloc(sizeof(t_mmu_page_read_response));
  package->buffer->offset = 0;

  response->page_size = buffer_read_uint32(package->buffer);
  response->page_data = safe_malloc(response->page_size);
  buffer_read(package->buffer, response->page_data, response->page_size);

  return response;
}

// sent by memory to cpu/mmu
t_package *create_mmu_page_read_response(void *page_data, uint32_t page_size)
{
  t_buffer *buffer = buffer_create(sizeof(uint32_t) + page_size);
  buffer_add_uint32(buffer, page_size);
  buffer_add(buffer, page_data, page_size);
  return package_create(READ_MEMORY, buffer);
}

int send_mmu_page_read_response(int socket, void *page_data, uint32_t page_size)
{
  t_package *package = create_mmu_page_read_response(page_data, page_size);
  int bytes_sent = send_package(socket, package);
  package_destroy(package);
  return bytes_sent;
}

void destroy_mmu_page_read_response(t_mmu_page_read_response *response)
{
  if (response)
  {
    free(response->page_data);
    free(response);
  }
}