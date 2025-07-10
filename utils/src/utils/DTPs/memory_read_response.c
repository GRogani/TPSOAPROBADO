#include "memory_read_response.h"

t_package *create_package_memory_read_response(char *data, uint32_t size)
{
  t_buffer *buffer = buffer_create(size + sizeof(uint32_t));
  buffer_add_string(buffer, size, data);
  t_package *package = package_create(READ_MEMORY, buffer);
  return package;
}

void send_memory_read_response(int socket, char *data, uint32_t size)
{
  t_package *package = create_package_memory_read_response(data, size);
  send_package(socket, package);
  package_destroy(package);
}

t_memory_read_response *read_memory_read_response(t_package *package)
{
  t_memory_read_response *response = safe_malloc(sizeof(t_memory_read_response));
  package->buffer->offset = 0;
  response->data = buffer_read_string(package->buffer, &response->data_size);
  return response;
}

void destroy_memory_read_response(t_memory_read_response *response)
{
  free(response->data);
  free(response);
}