#include "mmu_request_page_read_response.h"

// read by cpu/mmu
t_mmu_page_read_response *read_mmu_page_read_response(t_package *package)
{
  t_mmu_page_read_response *response = safe_malloc(sizeof(t_mmu_page_read_response));
  package->buffer->offset = 0;

  response->page_size = buffer_read_int32(package->buffer);
  response->page_data = safe_malloc(response->page_size);
  buffer_read(package->buffer, response->page_data, response->page_size);
  LOG_PACKAGE("Read MMU page read response: page_size: %u", response->page_size);
  return response;
}

// sent by memory to cpu/mmu
t_package *create_mmu_page_read_response(void *page_data, int32_t page_size)
{
  t_buffer *buffer = buffer_create(sizeof(int32_t) + page_size);
  buffer_add_int32(buffer, page_size);
  buffer_add(buffer, page_data, page_size);
  return create_package(READ_MEMORY, buffer);
}

int send_mmu_page_read_response(int socket, void *page_data, int32_t page_size)
{
  LOG_PACKAGE("Sending MMU page read response: page_size: %u", page_size);
  t_package *package = create_mmu_page_read_response(page_data, page_size);
  int bytes_sent = send_package(socket, package);
  destroy_package(package);
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