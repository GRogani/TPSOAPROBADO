#include "memory_read_response.h"

t_package *create_package_memory_read_response(char *data, int32_t size)
{
  t_buffer *buffer = buffer_create(size + sizeof(int32_t));
  buffer_add_string(buffer, size, data);
  t_package *package = create_package(READ_MEMORY, buffer);
  return package;
}

void send_memory_read_response(int socket, char *data, int32_t size)
{
  LOG_PACKAGE("Sending memory read response: size: %u", size);
  t_package *package = create_package_memory_read_response(data, size);
  send_package(socket, package);
  destroy_package(package);
}

t_memory_read_response *read_memory_read_response(t_package *package)
{
  t_memory_read_response *response = safe_malloc(sizeof(t_memory_read_response));
  package->buffer->offset = 0;
  response->data = buffer_read_string(package->buffer, &response->data_size);
  LOG_PACKAGE("Read memory read response: data_size: %u", response->data_size);
  return response;
}

void destroy_memory_read_response(t_memory_read_response *response)
{
  free(response->data);
  free(response);
}