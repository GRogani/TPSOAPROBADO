#include "io_syscall.h"

void handle_io_process_syscall(int32_t pid, int32_t pc, int32_t sleep_time, char *device_name)
{
  lock_io_connections();

  t_io_connection_status io_connection = get_io_connection_status_by_device_name(device_name);
  if (!io_connection.found)
  {
    handle_io_connection_not_found(pid, sleep_time, device_name);
    unlock_io_connections();
    return;
  }

  lock_io_requests_link();

  void *io_request_link = find_io_request_by_device_name(device_name);
  if (io_request_link == NULL)
  {
    LOG_ERROR("No IO request link found for device %s", device_name);
    unlock_io_requests_link();
    unlock_io_connections();
    return;
  }
  t_io_requests_link *io_request = (t_io_requests_link *)io_request_link;

  lock_io_requests_queue(&io_request->io_requests_queue_semaphore);
  lock_exec_list();
  lock_blocked_list();

  t_pcb *pcb = remove_pcb_from_exec(pid);
  if (pcb == NULL)
  {
    LOG_ERROR("PCB with PID %d not found in EXEC list", pid);
    unlock_blocked_list();
    unlock_exec_list();
    unlock_io_requests_queue(&io_request->io_requests_queue_semaphore);
    unlock_io_requests_link();
    unlock_io_connections();
    return;
  }

  pcb->pc = pc;
  add_pcb_to_blocked(pcb);
  create_io_request_element(io_request->io_requests_queue, pid, sleep_time);

  LOG_OBLIGATORIO("## (%d) - Bloqueado por IO: %s", pid, device_name);
  unlock_blocked_list();
  unlock_exec_list();
  unlock_io_requests_queue(&io_request->io_requests_queue_semaphore);
  unlock_io_requests_link();
  unlock_io_connections();

  t_pending_io_args pending_io_args;
  pending_io_args.device_name = strdup(device_name);
  pending_io_args.client_socket = io_connection.socket_id;
  process_pending_io(pending_io_args);

  // TODO: testear esto de abajo
  pthread_t thread;
  LOG_INFO("io_syscall: Attempting to run the medium-term scheduler");
  if(pthread_create(&thread, NULL, medium_scheduler_thread,(void*)(uintptr_t)pcb->pid) != 0){
    LOG_ERROR("Failure pthread_create, PID [%u]", pcb->pid);
  }
  pthread_detach(thread);

  pthread_t short_scheduler_t;
  LOG_INFO("io_syscall: Attempting to run the short-term scheduler");
  if (pthread_create(&short_scheduler_t, NULL, short_scheduler_thread, NULL) != 0)
  {
    LOG_ERROR("Failure pthread_create, PID [%u]", pcb->pid);
  }
  pthread_detach(short_scheduler_t);
}

void short_scheduler_thread()
{
  LOG_INFO("io_syscall: running short scheduler");
  run_short_scheduler();
  LOG_INFO("io_syscall: short scheduler finished");
}

void handle_io_connection_not_found(int32_t pid, int32_t sleep_time, char *device_name)
{
  LOG_INFO("IO connection not found for device %s in PID %d", device_name, pid);

  lock_exec_list();

  t_pcb *pcb = remove_pcb_from_exec(pid);

  bool memory_space_free = exit_routine(pcb);
  unlock_exec_list();

  pthread_t thread;
  if (pthread_create(&thread, NULL, process_schedulers_io, NULL) != 0)
  {
    LOG_ERROR("IO syscall: Failure pthread_create");
  }
  pthread_detach(thread);
}

void process_schedulers_io()
{
  run_long_scheduler();
  run_short_scheduler();
}

void* medium_scheduler_thread(void* arg) {
    int32_t pid = (int32_t)(uintptr_t)arg;
    run_medium_scheduler(pid);
    return NULL;
}