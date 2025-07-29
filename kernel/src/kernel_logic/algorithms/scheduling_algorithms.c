#include "scheduling_algorithms.h"

static PLANIFICATION_ALGORITHM short_term_algorithm;
static PLANIFICATION_ALGORITHM long_term_algorithm;
static bool preemption_enabled;

t_pcb* get_next_process_to_initialize_from_new(void) 
{
    if (long_term_algorithm == PMCP)
        sort_new_list_by_SSF();

    return get_next_pcb_from_new();
}

t_pcb* get_next_process_to_initialize_from_susp_ready(void) 
{
    if (long_term_algorithm == PMCP)
        sort_susp_ready_list_by_SSF();

    return get_next_pcb_from_susp_ready();
}


t_cpu_connection* get_cpu_by_algorithm(t_list *cpus)
{
    t_cpu_connection* cpu = NULL;
    
    if (short_term_algorithm == SJF)
        cpu = get_cpu_by_SJF (cpus);
    else
        cpu = (t_cpu_connection*)list_get(cpus, 0); // FIFO
    
    return cpu;
}

t_pcb* get_next_process_to_dispatch(void) 
{
    if (short_term_algorithm == SJF)     
        sort_ready_list_by_SJF();

    return get_next_pcb_from_ready();
    
}

bool preemption_is_enabled(void) 
{
    return preemption_enabled;
}

bool should_preempt_executing_process(t_pcb *pcb_ready, int32_t pid_executing)
{
    lock_exec_list();
    t_pcb *pcb_executing = find_pcb_in_exec(pid_executing);
    if(pcb_executing == NULL)  {
        // aca podria ser que se ejecutó una syscall y el proceso salió de la lista de EXEC. deberiamos hacer el preemption, porque el proceso se libero de la lista EXEC.
        return true;
    }
    return compare_cpu_bursts_SRT((void *)pcb_ready, (void *)pcb_executing);
}

void configure_scheduling_algorithms() 
{   
    extern t_kernel_config kernel_config; // en globals.h

    short_term_algorithm = kernel_config.short_planification_algorithm;
    long_term_algorithm = kernel_config.long_planification_algorithm;
    preemption_enabled = kernel_config.preemption_enabled;
}
