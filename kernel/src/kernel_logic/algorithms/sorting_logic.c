#include "scheduling_algorithms.h"

// SFJ (SHORT PLANNING)

bool compare_cpu_bursts(void *a, void *b)
{
    extern t_kernel_config kernel_config; // en globals.h
    double alpha = kernel_config.alpha;

    t_pcb *pcb_a = (t_pcb *)a;
    t_pcb *pcb_b = (t_pcb *)b;

    // Est(n+1) =  R(n) + (1-) Est(n) ;

    pcb_a->MT.last_estimated_cpu_burst_ms = pcb_a->MT.last_estimated_cpu_burst_ms * (1 - alpha) + pcb_a->MT.last_cpu_burst_ms * alpha;
    pcb_b->MT.last_estimated_cpu_burst_ms = pcb_b->MT.last_estimated_cpu_burst_ms * (1 - alpha) + pcb_b->MT.last_cpu_burst_ms * alpha;

    LOG_INFO("Comparando %d con %d: Estimación A: %d, Estimación B: %d",
             pcb_a->pid, pcb_b->pid,
             pcb_a->MT.last_estimated_cpu_burst_ms,
             pcb_b->MT.last_estimated_cpu_burst_ms);

    return pcb_a->MT.last_estimated_cpu_burst_ms <= pcb_b->MT.last_estimated_cpu_burst_ms;
}
// SRT
bool compare_cpu_bursts_SRT(void *a, void *b)
{
    extern t_kernel_config kernel_config; // en globals.h
    double alpha = kernel_config.alpha;

    t_pcb *pcb_a = (t_pcb *)a;
    t_pcb *pcb_b = (t_pcb *)b;

    // Est(n+1) =  R(n) + (1-) Est(n) ;

    pcb_a->MT.last_estimated_cpu_burst_ms = pcb_a->MT.last_estimated_cpu_burst_ms * (1 - alpha) + pcb_a->MT.last_cpu_burst_ms * alpha;
    pcb_b->MT.last_estimated_cpu_burst_ms = pcb_b->MT.last_estimated_cpu_burst_ms * (1 - alpha) + pcb_b->MT.last_cpu_burst_ms * alpha;

    LOG_INFO("Comparando %d con %d: Estimación A: %d, Estimación B: %d",
                    pcb_a->pid, pcb_b->pid,
                    pcb_a->MT.last_estimated_cpu_burst_ms,
                    pcb_b->MT.last_estimated_cpu_burst_ms);

    int process_executing = (pcb_b->MT.last_estimated_cpu_burst_ms - total_time_ms(pcb_b->state_start_time_ms));
    int process_to_dispatch = pcb_a->MT.last_estimated_cpu_burst_ms;

    LOG_INFO("Comparando %d con %d: A: %d, B: %d", pcb_a->pid, pcb_b->pid, process_to_dispatch, process_executing);

    LOG_INFO("%d", process_executing <= process_to_dispatch);

    if (process_executing <= process_to_dispatch)
    {
        return false; // no debemos desalojar al proceso que está ejecutando actualmente
    }

    return true;
}

void sort_ready_list_by_SJF()
{
    list_sort(get_ready_list(), compare_cpu_bursts);
}

void sort_exec_list_by_SJF()
{
    list_sort(get_exec_list(), compare_cpu_bursts);
}

t_cpu_connection *get_cpu_by_SJF(t_list *cpus)
{
    sort_exec_list_by_SJF();
    int exec_list_size = list_size(get_exec_list());

    if (exec_list_size == 0)
    {
        LOG_DEBUG("No hay procesos en la lista de ejecución");
        return list_get(cpus, 0);
    }
    else
    {
        t_pcb *pcb = (t_pcb *)list_get(get_exec_list(), exec_list_size - 1);
        return (t_cpu_connection *)get_cpu_connection_by_pid(pcb->pid);
    }
}

// SHORTEST SIZE FIRST (LONG PLANNING)

bool compare_process_size(void *a, void *b)
{
    t_pcb *pcb_a = (t_pcb *)a;
    t_pcb *pcb_b = (t_pcb *)b;

    return pcb_a->size <= pcb_b->size;
}

void sort_new_list_by_SSF()
{
    list_sort(get_new_list(), compare_process_size);
}

void sort_susp_ready_list_by_SSF()
{
    list_sort(get_susp_ready_list(), compare_process_size);
}