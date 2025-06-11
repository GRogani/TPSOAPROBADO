#include "dump_memory_syscall.h"

void dump_memory_syscall(uint32_t pid) 
{
    LOG_INFO("Dump memory syscall called for PID %d", pid);
    
    lock_exec_list();
        t_pcb *pcb = (t_pcb*)find_pcb_in_exec(pid);
        if (pcb == NULL) {
            LOG_ERROR("No process found with PID %d", pid);
            unlock_exec_list();
            return;
        }
        remove_pcb_from_exit(pid);
    unlock_exec_list();

    lock_blocked_list();
        add_pcb_to_blocked(pcb);
    unlock_blocked_list();
    
    int confirmation = dump_memory_routine(pid);

    if (confirmation == 0)
    {
        lock_blocked_list(pcb);
            remove_pcb_from_blocked(pcb);
        unlock_blocked_list(pcb);

        lock_ready_list();
            add_pcb_to_ready(pcb);
        unlock_ready_list();

        run_short_scheduler();
    }
    else
    {
        LOG_ERROR("Failed to dump memory for PID %d", pid);

        lock_blocked_list(pcb);
            remove_pcb_from_blocked(pcb);
        unlock_blocked_list(pcb);

        exit_routine(pcb);
    }



}