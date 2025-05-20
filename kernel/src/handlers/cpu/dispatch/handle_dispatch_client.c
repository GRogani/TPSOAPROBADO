#include "handle_dispatch_client.h"

void *handle_dispatch_client(void* arg)
{
  char* connection_id = (char *) arg;

  t_cpu_connection* cpu_connection = get_cpu_connection_by_id(connection_id);
  if (cpu_connection == NULL)
  {
    log_error(get_logger(), "CPU connection not found for id %s", connection_id);
    pthread_exit(0);
    return;
  }

  t_package *package;
  while (1)
  {
    package = recv_package(cpu_connection->dispatch_socket_id);
    if (package != NULL)
    {
      switch (package->opcode)
      {
      case HANDSHAKE:
      {
        // TODO: devolvemos un paquete dummy
        log_info(get_logger(), "CPU HANDSHAKE RECEIVED for connection with id -> %s", connection_id);
        break;
      }
      case SYSCALL:
      {
        // TODO: leemos el paquete, verificamos que tipo de syscall es.
        // dependiendo del tipo, handeleamos.
        break;
      }
      default:
      {
        log_error(get_logger(), "Unknown opcode %d from CPU", package->opcode);
        package_destroy(package);        
        break;
      }
      }
    }
    else
    {
      log_error(get_logger(), "CPU %s disconnected", connection_id);
      exit(EXIT_FAILURE);
      return;
    }
  }
}