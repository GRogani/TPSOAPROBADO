#include "memory_write_request.h"
#include "utils/safe_alloc.h"
#include <string.h>

t_memory_write_request *create_memory_write_request(uint32_t physical_address, uint32_t size, void *data)
{
  t_memory_write_request *request = safe_malloc(sizeof(t_memory_write_request));
  request->physical_address = physical_address;
  request->size = size;
  request->data = safe_malloc(size);
  memcpy(request->data, data, size);
  return request;
}

void destroy_memory_write_request(t_memory_write_request *request)
{
  free(request->data);
  free(request);
}

int send_memory_write_request(int socket, t_memory_write_request *request)
{
  t_buffer *buffer = buffer_create(sizeof(uint32_t) * 2 + request->size);
  buffer_add_uint32(buffer, request->physical_address);
  buffer_add_uint32(buffer, request->size);
  buffer_add(buffer, request->data, request->size);
  t_package *package = create_package(WRITE_MEMORY, buffer);
  int result = send_package(socket, package);
  destroy_package(package);
  return result;
}

t_memory_write_request *read_memory_write_request(t_package *package)
{
  uint32_t physical_address;
  uint32_t size;
  void *data;

  package->buffer->offset = 0;
  physical_address = buffer_read_uint32(package->buffer);
  size = buffer_read_uint32(package->buffer);
  data = safe_malloc(size);
  buffer_read(package->buffer, data, size);

  t_memory_write_request *request = create_memory_write_request(physical_address, size, data);
  free(data);
  return request;
}