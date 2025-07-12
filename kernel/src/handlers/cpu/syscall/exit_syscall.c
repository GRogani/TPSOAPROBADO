#include "exit_syscall.h"

void exit_process_syscall(uint32_t pid) 
{
  LOG_INFO("Exit syscall called for PID %d", pid);

  lock_exec_list();

  t_pcb *pcb = remove_pcb_from_exec(pid);

  bool memory_space_free = exit_routine(pcb);
  unlock_exec_list();

  if (memory_space_free) // si no se pudo sacar de la memoria, no tenemos que correr esto, no tiene sentido porque no se liberó memoria, quedo el proceso ahi zombie.
  {
    run_long_scheduler();
  }

  run_short_scheduler(); // si o si lo corremos, porque el proceso pasó a EXIT y tenemos que replanificar.
}

bool exit_routine(t_pcb* pcb) {
  bool memory_space_free = false;
  lock_exit_list();
  LOG_INFO("Lockeada lista EXIT");

  add_pcb_to_exit(pcb); // lo pasamos a EXIT

  int mem_response = kill_process_in_memory(pcb->pid);
  LOG_INFO("KILL PROCESS IN MEMORY EXECUTED");

  if (mem_response == 0)
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

void log_process_metrics(uint32_t pid, t_state_metrics state_metrics, t_time_metrics time_metrics)
{
  LOG_OBLIGATORIO("## (%d) - Metricas de estado: NEW (%d) (%ldms), READY (%d) (%ldms),  SUSPEND REDY (%d) (%ldms), EXEC (%d) (%ldms), BLOCK (%d) (%ldms), SUSPEND BLOCK (%d) (%ldms), EXIT (%d) (%ldms)", 
          pid, 
          state_metrics.new_count          , time_metrics.new_time_ms, 
          state_metrics.ready_count        , time_metrics.ready_time_ms,
          state_metrics.susp_ready_count   , time_metrics.susp_ready_time_ms,
          state_metrics.exec_count         , time_metrics.exec_time_ms, 
          state_metrics.blocked_count        , time_metrics.blocked_time_ms, 
          state_metrics.susp_blocked_count , time_metrics.susp_blocked_time_ms,
          state_metrics.exit_count         , time_metrics.exit_time_ms
        );
}