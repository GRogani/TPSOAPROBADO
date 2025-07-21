#include "kill_process_package.h"

t_package *create_kill_process_package(uint32_t pid)
{
  t_buffer *buffer;
  buffer = buffer_create(sizeof(uint32_t));
  buffer_add_uint32(buffer, pid);
  return create_package(KILL_PROCESS, buffer);
}

int send_kill_process_package(int socket, uint32_t pid)
{
  LOG_PACKAGE("Sending kill process package: pid: %u", pid);
  t_package *package = create_kill_process_package(pid);
  int bytes_sent = send_package(socket, package);
  destroy_package(package);
  return bytes_sent;
}

uint32_t read_kill_process_package(t_package *package)
{
  package->buffer->offset = 0;
  uint32_t pid = buffer_read_uint32(package->buffer);
  package->buffer->offset = 0;
  LOG_PACKAGE("Read kill process package: pid: %u", pid);
  return pid;
}