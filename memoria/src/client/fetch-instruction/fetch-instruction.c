#include "fetch-instruction.h"

void get_instruction_request_handler(int socket, t_package *package)
{
  fetch_package_data request = read_fetch_package(package);
  int32_t pid = request.pid;
  int32_t pc = request.pc;

  process_info *proc = process_manager_find_process(pid);

  if (proc != NULL)
  {
    lock_process_metrics();
    proc->metrics->instruction_requests_count++;
    unlock_process_metrics();

    lock_process_instructions();
    if (proc->instructions && pc < list_size(proc->instructions))
    {
      char *instruction = list_get(proc->instructions, pc);
      if (instruction)
      {
        LOG_OBLIGATORIO("## PID: %u - Obtener instrucción: %u - Instrucción: %s", pid, pc, instruction);
        send_instruction_package(socket, instruction);
      }
      else
      {
        LOG_ERROR("## PID: %u - PC %u la instruccion es NULL.", pid, pc);
        send_instruction_package(socket, "");
      }
    }
    else
    {
      LOG_ERROR("## PID: %u - PC %u fuera de limites (max: %d).", pid, pc,
                proc->instructions ? list_size(proc->instructions) : 0);
      send_instruction_package(socket, "");
    }
    unlock_process_instructions();
  }
  else
  {
    LOG_ERROR("## PID: %u - Proceso no encontrado.", pid);
    send_instruction_package(socket, "");
  }
}
