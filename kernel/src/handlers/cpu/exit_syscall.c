#include "exit_syscall.h"

void exit_process_syscall(uint32_t pid) 
{
  t_pcb* pcb;

  lock_exec_list();
  lock_exit_list();

  pcb = remove_pcb_from_exec(pid);
  add_pcb_to_exit(pcb);

  //dump_memory(pid);
  LOG_WARNING("Implementar dump_memory()");

  log_process_metrics(pid ,pcb->ME, pcb->MT);

  unlock_exec_list();
  unlock_exit_list();

  run_long_scheduler();
  run_short_scheduler();

}

void log_process_metrics(uint32_t pid, t_state_metrics state_metrics, t_time_metrics time_metrics)
{
  LOG_INFO("## (%d) - Metricas de estado: NEW (%d) (%dms), READY (%d) (%dms),  SUSPEND REDY (%d) (%dms), EXEC (%d) (%dms), BLOCK (%d) (%dms), SUSPEND BLOCK (%d) (%dms), EXIT (%d) (%dms)", 
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