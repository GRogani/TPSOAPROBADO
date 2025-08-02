#include "exit_syscall.h"

void exit_process_syscall(int32_t pid)
{
  LOG_INFO("Exit syscall called for PID %d", pid);

  lock_exec_list();

  t_pcb *pcb = remove_pcb_from_exec(pid);

  bool memory_space_free = exit_routine(pcb);
  unlock_exec_list();

  pthread_t thread;
  if (pthread_create(&thread, NULL, process_schedulers_exit, NULL) != 0)
  {
    LOG_ERROR("EXIT syscall: Failure pthread_create");
  }
  pthread_detach(thread);
}

void process_schedulers_exit()
{
  LOG_INFO("exit_syscall: Ejecutando planificador de largo plazo");
  run_long_scheduler();
  LOG_INFO("exit_syscall: Planificador de largo plazo completado");

  LOG_INFO("exit_syscall: Ejecutando planificador de corto plazo");
  run_short_scheduler();
  LOG_INFO("exit_syscall: Syscall EXIT completada");
}

bool exit_routine(t_pcb* pcb) {
  bool memory_space_free = false;

  lock_exit_list();
  add_pcb_to_exit(pcb);

  bool mem_response = kill_process_in_memory(pcb->pid);
  LOG_INFO("KILL PROCESS IN MEMORY EXECUTED");

  if (mem_response)
  {
    LOG_OBLIGATORIO("## (%d) - Finaliza el proceso", pcb->pid);
    memory_space_free = true;
    pcb->MT.exit_time_ms = total_time_ms(pcb->state_start_time_ms);
  }
  else
  {
    LOG_WARNING("## (%d) - Proceso Zombie", pcb->pid);
    pcb->MT.exec_time_ms = get_current_time_ms();
  }

  LOG_INFO("LOGEANDO METRICAS...");
  log_process_metrics(pcb->pid, pcb->ME, pcb->MT);

  unlock_exit_list();
  LOG_INFO("EXIT DESLOCKEADO");

  return memory_space_free;
}

void log_process_metrics(int32_t pid, t_state_metrics state_metrics, t_time_metrics time_metrics)
{
  char padding[] = "                                            ";
  LOG_OBLIGATORIO("## (%d) - Metricas de estado: \n%sNEW (%d) (%ldms), \n%sREADY (%d) (%ldms), \n%sSUSPEND REDY (%d) (%ldms), \n%sEXEC (%d) (%ldms), \n%sBLOCK (%d) (%ldms), \n%sSUSPEND BLOCK (%d) (%ldms), \n%sEXIT (%d) (%ldms)",
                  pid,
                  padding ,state_metrics.new_count, time_metrics.new_time_ms,
                  padding ,state_metrics.ready_count, time_metrics.ready_time_ms,
                  padding ,state_metrics.susp_ready_count, time_metrics.susp_ready_time_ms,
                  padding ,state_metrics.exec_count, time_metrics.exec_time_ms,
                  padding ,state_metrics.blocked_count, time_metrics.blocked_time_ms,
                  padding ,state_metrics.susp_blocked_count, time_metrics.susp_blocked_time_ms,
                  padding ,state_metrics.exit_count, time_metrics.exit_time_ms);
}