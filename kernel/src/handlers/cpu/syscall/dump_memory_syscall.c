#include "dump_memory_syscall.h"

void dump_memory_syscall(int32_t pid, int32_t pc)
{
    LOG_INFO("Dump memory syscall called for PID %d", pid);

    lock_exec_list();
    lock_blocked_list();

    t_pcb *pcb = (t_pcb *)find_pcb_in_exec(pid);
    if (pcb == NULL)
    {
        LOG_ERROR("No process found with PID %d", pid);
        unlock_exec_list();
        unlock_blocked_list();
        return;
    }
    pcb->pc = pc;
    remove_pcb_from_exec(pid);
    add_pcb_to_blocked(pcb);

    unlock_blocked_list();
    unlock_exec_list();

    bool confirmation = dump_memory_routine(pid);

    lock_ready_list();
    lock_blocked_list();

    if (confirmation)
    {
        
        remove_pcb_from_blocked(pcb->pid);
        add_pcb_to_ready(pcb);
        
        
        unlock_ready_list();
        unlock_blocked_list();

        run_short_scheduler();
    }
    else
    {
        LOG_ERROR("Failed to dump memory for PID %d", pid);

        remove_pcb_from_blocked(pcb->pid);

        exit_routine(pcb);

        unlock_ready_list();
        unlock_blocked_list();
    }
}