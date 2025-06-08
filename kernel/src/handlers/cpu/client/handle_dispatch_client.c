#include "handle_dispatch_client.h"

void *handle_dispatch_client(void *arg)
{
  char *connection_id = (char *)arg;

  t_cpu_connection *cpu_connection = get_cpu_connection_by_id(connection_id);
  if (cpu_connection == NULL)
  {
    LOG_ERROR("CPU connection not found for id %s", connection_id);
    pthread_exit((void *)-1);
  }

  t_package *package;
  while (1)
  {
    package = recv_package(cpu_connection->dispatch_socket_id);
    if (package != NULL)
    {
      switch (package->opcode)
      {
      case SYSCALL:
      {
        handle_cpu_syscall(package, cpu_connection->dispatch_socket_id);
        destroy_package(package);
        break;
      }
      default:
      {
        LOG_ERROR("Unknown opcode %d from CPU", package->opcode);
        destroy_package(package);
        break;
      }
      }
    }
    else
    {
      LOG_ERROR("CPU %s disconnected", connection_id);
      // exit(EXIT_FAILURE);
      pthread_exit((void *)1);
    }
  }
}

void handle_cpu_syscall(t_package *package, int socket)
{
  syscall_package_data *syscall = read_syscall_package(package);

  if (syscall == NULL)
  {
    LOG_ERROR("Failed to read CPU syscall");
    return;
  }

  LOG_INFO("Processing CPU syscall type %d for PID %d",
           syscall->syscall_type, syscall->pid);

  switch (syscall->syscall_type)
  {
  case SYSCALL_INIT_PROC:
  {
    // Execute init_proc syscall
    handle_init_proc_syscall(syscall->pid,
                             syscall->params.init_proc.memory_space,
                             syscall->params.init_proc.pseudocode_file);

    if (syscall->pid != 0)
    {
      LOG_INFO("Sending syscall response for init_proc PID %d", syscall->pid);
      send_confirmation_package(socket, 0); // 0 = success
    }
    break;
  }
  case SYSCALL_IO:
  {
    handle_io_process_syscall(syscall->pid,
                              syscall->params.io.sleep_time,
                              syscall->params.io.device_name);
    break;
  }
  case SYSCALL_DUMP_PROCESS:
  {
    // TODO: Handle dump process syscall
    LOG_INFO("Dump process syscall not implemented yet");
    break;
  }
  case SYSCALL_EXIT:
  {
    exit_process_syscall(syscall->pid);
    break;
  }
  default:
  {
    LOG_ERROR("Unknown syscall type: %d", syscall->syscall_type);
    break;
  }
  }

  destroy_syscall_package(syscall);
}