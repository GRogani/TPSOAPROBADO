#include "handle_dispatch_client.h"

void *handle_dispatch_client(void* arg)
{
  char* connection_id = (char *) arg;

  t_cpu_connection* cpu_connection = get_cpu_connection_by_id(connection_id);
  if (cpu_connection == NULL)
  {
    log_error(get_logger(), "CPU connection not found for id %s", connection_id);
    pthread_exit(0);
    return NULL;
  }

  t_package *package;
  while (1)
  {
    package = recv_package(cpu_connection->dispatch_socket_id);
    if (package != NULL)
    {
      switch (package->opcode)
      {
      case CPU_SYSCALL:
      {
        handle_cpu_syscall(package, cpu_connection->dispatch_socket_id);
        package_destroy(package);
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
      return NULL;
    }
  }
}

void handle_cpu_syscall(t_package* package, int socket) 
{
  t_cpu_syscall *syscall = read_cpu_syscall_request(package);

  if (syscall == NULL) {
    log_error(get_logger(), "Failed to read CPU syscall");
    return;
  }
  
  log_info(get_logger(), "Processing CPU syscall type %d for PID %d", 
           syscall->syscall_type, syscall->pid);
  
  switch (syscall->syscall_type) {
    case SYSCALL_INIT_PROC: {
      // Execute init_proc syscall
      handle_init_proc_syscall(syscall->pid, 
                              syscall->params.init_proc.memory_space,
                              syscall->params.init_proc.pseudocode_file);
      
      if (syscall->pid != 0) {
        log_info(get_logger(), "Sending syscall response for init_proc PID %d", syscall->pid);
        send_cpu_syscall_response(socket, 0); // 0 = success
      }
      break;
    }
    case SYSCALL_IO: {
      // TODO: Handle IO syscall
      log_info(get_logger(), "IO syscall not implemented yet");
      break;
    }
    case SYSCALL_DUMP_PROCESS: {
      // TODO: Handle dump process syscall
      log_info(get_logger(), "Dump process syscall not implemented yet");
      break;
    }
    case SYSCALL_EXIT: {
      // TODO: Handle exit syscall
      log_info(get_logger(), "Exit syscall not implemented yet");
      break;
    }
    default: {
      log_error(get_logger(), "Unknown syscall type: %d", syscall->syscall_type);
      break;
    }
  }
  
  destroy_cpu_syscall(syscall);
}