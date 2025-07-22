#include "memory_read_request.h"

t_memory_read_request *create_memory_read_request(int32_t physical_address, int32_t size)
{
  t_memory_read_request *request = safe_malloc(sizeof(t_memory_read_request));
  request->physical_address = physical_address;
  request->size = size;
  return request;
}

void destroy_memory_read_request(t_memory_read_request *request)
{
  free(request);
}

void send_memory_read_request(int socket, t_memory_read_request *request)
{
  LOG_PACKAGE("Sending memory read request: physical_address: %u, size: %u", request->physical_address, request->size);
  t_buffer *buffer = buffer_create(2 * sizeof(int32_t));
  buffer_add_int32(buffer, request->physical_address);
  buffer_add_int32(buffer, request->size);
  t_package *package = create_package(READ_MEMORY, buffer);
  send_package(socket, package);
  destroy_package(package);
}

t_memory_read_request *read_memory_read_request(t_package *package)
{
  int32_t physical_address;
  int32_t size;

  package->buffer->offset = 0;
  physical_address = buffer_read_int32(package->buffer);
  size = buffer_read_int32(package->buffer);

  t_memory_read_request *request = create_memory_read_request(physical_address, size);
  LOG_PACKAGE("Read memory read request: physical_address: %u, size: %u", request->physical_address, request->size);
  return request;
}