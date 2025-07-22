#include "kill-process.h"

void delete_process_request_handler(int socket, t_package *package)
{
  int32_t pid_to_delete = read_kill_process_package(package);

  LOG_INFO("## PID: %u - Solicitud de Finalizacion de Proceso Recibida.", pid_to_delete);

  bool result = process_manager_delete_process(pid_to_delete);

  if (result)
  {
    LOG_OBLIGATORIO("## PID: %u - Proceso Finalizado y recursos liberados.", pid_to_delete);
    send_confirmation_package(socket, true);
  }
  else
  {
    LOG_ERROR("## PID: %u - Intento de finalizar proceso no existente.", pid_to_delete);
    send_confirmation_package(socket, false);
  }
}
