#include "mmu_request_page_read_from_memory.h"

// read from cpu/mmu
t_mmu_page_read_request *read_mmu_page_read_request(t_package *package)
{
  package->buffer->offset = 0;
  t_mmu_page_read_request *request = safe_malloc(sizeof(t_mmu_page_read_request));

  request->frame_number = buffer_read_uint32(package->buffer);

  return request;
}

// sent by cpu/mmu to memory
t_package *create_mmu_page_read_request(uint32_t frame_number)
{
  t_buffer *buffer = buffer_create(sizeof(uint32_t));
  buffer_add_uint32(buffer, frame_number);
  return package_create(MMU_PAGE_READ_REQUEST, buffer);
}

int send_mmu_page_read_request(int socket, uint32_t frame_number)
{
  t_package *package = create_mmu_page_read_request(frame_number);
  int bytes_sent = send_package(socket, package);
  package_destroy(package);
  return bytes_sent;
}

void destroy_mmu_page_read_request(t_mmu_page_read_request *request)
{
  if (request)
  {
    free(request);
  }
}