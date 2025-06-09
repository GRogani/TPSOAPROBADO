#include "io_disconnected.h"

void io_disconnected(int socket_id)
{
  lock_io_connections();

  t_io_connection *io_connection = find_io_connection_by_socket(socket_id);
  if (io_connection == NULL)
  {
    // No connection found for the given socket_id
    LOG_ERROR("No IO connection found for socket %d but it was disconnected", socket_id);
    unlock_io_connections();
    pthread_exit(0);
    return;
  }

  LOG_INFO("Client disconnected %s", io_connection->device_name);

  if (io_connection->current_process_executing != -1)
  {
    LOG_INFO("Process %d was executing on device %s but it was disconnected.", io_connection->current_process_executing, io_connection->device_name);
    handle_found_process(io_connection);
  }

  LOG_INFO("Deleting IO connection for socket %d", socket_id);

  delete_io_connection(socket_id);
  unlock_io_connections();
  pthread_exit(0);
}

void handle_found_process(t_io_connection *io_connection)
{
  lock_blocked_list();
  t_pcb *pcb = remove_pcb_from_blocked(io_connection->current_process_executing);
  if (pcb != NULL)
  {
    // fué removido con exito, corremos rutina final
    exit_routine(pcb);
    unlock_blocked_list();
    return;
  }
  unlock_blocked_list();

  lock_susp_blocked_list();
  pcb = remove_pcb_from_susp_blocked(io_connection->current_process_executing);
  if (pcb != NULL)
  {
    // fué removido con exito, corremos rutina final
    exit_routine(pcb);
    unlock_susp_blocked_list();
    return;
  }
  // Si no existe en ninguna de las listas, loggeamos un error.
  LOG_ERROR("Process %d was not found in any blocked list", io_connection->current_process_executing);

  unlock_susp_blocked_list();
}