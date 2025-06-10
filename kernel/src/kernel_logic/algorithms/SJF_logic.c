#include "scheduling_algorithms.h"

bool compare_cpu_bursts(void *a, void *b) 
{
    extern t_kernel_config kernel_config; // en globals.h
    double alpha = kernel_config.alpha;

    t_pcb *pcb_a = (t_pcb *)a;
    t_pcb *pcb_b = (t_pcb *)b;

    //Est(n+1) =  R(n) + (1-) Est(n) ;

    pcb_a->MT.last_estimated_cpu_busrt_ms = pcb_a->MT.last_estimated_cpu_busrt_ms * (1 - alpha) + pcb_a->MT.last_cpu_burst_ms * alpha;
    pcb_b->MT.last_estimated_cpu_busrt_ms = pcb_b->MT.last_estimated_cpu_busrt_ms * (1 - alpha) + pcb_b->MT.last_cpu_burst_ms * alpha;

    return pcb_a->MT.last_estimated_cpu_busrt_ms < pcb_b->MT.last_estimated_cpu_busrt_ms;

}

void sort_ready_list_by_SJF()
{
    lock_ready_list();

    list_sort ( get_ready_list(), compare_cpu_bursts );
    
    unlock_ready_list();
}

void sort_exec_list_by_SJF()
{
    lock_exec_list();

    list_sort ( get_exec_list(), compare_cpu_bursts );

    unlock_exec_list();
}

void sort_susp_ready_list_by_SJF()
{
    lock_susp_ready_list();

    list_sort ( get_susp_ready_list(), compare_cpu_bursts );

    unlock_susp_ready_list();
}

void sort_cpu_list_by_SJF(t_list *cpus)
{
    lock_exec_list();
    sort_exec_list_by_SJF();
}

