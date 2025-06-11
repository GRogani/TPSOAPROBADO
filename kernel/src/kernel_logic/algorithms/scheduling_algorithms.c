#include "scheduling_algorithms.h"

static PLANIFICATION_ALGORITHM short_term_algorithm;
static PLANIFICATION_ALGORITHM long_term_algorithm;
static bool preemption_enabled;

t_pcb* get_next_process_to_initialize_from_new(void) 
{
    if (long_term_algorithm == SJF)
            sort_new_list_by_SSF();

    return get_next_pcb_from_new();
}

t_pcb* get_next_process_to_initialize_from_susp_ready(void) 
{
    if (long_term_algorithm == SJF)
        sort_susp_ready_list_by_SSF();

    return get_next_pcb_from_susp_ready();
}


t_cpu_connection* get_cpu_by_algorithm(t_list *cpus)
{
    if (short_term_algorithm == SJF)
        get_cpu_by_SJF (cpus);

    return (t_cpu_connection*)list_get(cpus, 0);
}

t_pcb* get_next_process_to_dispatch(void) 
{
    if (short_term_algorithm == SJF)     
        sort_ready_list_by_SJF();

    return get_next_pcb_from_ready();
    
}

bool preepmtion_is_enabled(void) 
{
    return preemption_enabled;
}

bool should_preempt_executing_process(t_pcb* pcb_ready, t_pcb *pid_executing)
{
    if (!preepmtion_is_enabled())
        return false;
    else
        return compare_cpu_bursts( (void*)pcb_ready, (void*)pid_executing);
}

void configure_scheduling_algorithms() 
{   
    extern t_kernel_config kernel_config; // en globals.h

    short_term_algorithm = kernel_config.short_planification_algorithm;
    long_term_algorithm = kernel_config.long_planification_algorithm;
    preemption_enabled = kernel_config.preemption_enabled;
}
