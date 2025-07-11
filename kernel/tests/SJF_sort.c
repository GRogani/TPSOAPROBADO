#include "../utils.h"
#include "kernel_logic/algorithms/scheduling_algorithms.h"
#include "semaphore/semaphore.h"
#include "globals.h"


void log_ready_list(const char* title) {
    LOG_INFO("%s", title);
    t_list* ready_list = get_ready_list();
    for (int i = 0; i < list_size(ready_list); i++) {
        t_pcb* pcb = list_get(ready_list, i);
        LOG_INFO("PCB PID=%u | Estimado=%lu", 
            pcb->pid, pcb->MT.last_estimated_cpu_burst_ms);
    }
    LOG_INFO("Total PCBs in ready list: %d", list_size(ready_list));

}


int main() 
{  
    extern t_kernel_config kernel_config;// en globals.h
    t_config* config = init_config("kernel.config");
    kernel_config = init_kernel_config(config);
    initialize_global_lists(); 
    initialize_global_semaphores();
    init_logger("kernel.log", "[TEST]", kernel_config.log_level);

    // Crear PCBs con IDs distintos para trazabilidad
    t_pcb *pcb1 = pcb_create(1, 100, 1, "test1.pseudocode");
    t_pcb *pcb2 = pcb_create(2, 200, 2, "test2.pseudocode");
    t_pcb *pcb3 = pcb_create(3, 150, 3, "test3.pseudocode");

    // Estimaciones iniciales
    pcb1->MT.last_estimated_cpu_burst_ms = 100;
    pcb1->MT.last_cpu_burst_ms = 120;
    pcb2->MT.last_estimated_cpu_burst_ms = 80;
    pcb2->MT.last_cpu_burst_ms = 90;
    pcb3->MT.last_estimated_cpu_burst_ms = 150;
    pcb3->MT.last_cpu_burst_ms = 130;

    // Agregar a ready
    add_pcb_to_ready(pcb1);
    add_pcb_to_ready(pcb2);
    add_pcb_to_ready(pcb3);

    // Log antes de ordenar
    log_ready_list("Ready list before sorting:");

    // Ordenar con SJF
    sort_ready_list_by_SJF();

    // Log después de ordenar
    log_ready_list("Ready list after sorting:");

    // Obtener siguiente PCB
    t_pcb* next_pcb = get_next_pcb_from_ready();
    LOG_INFO("Next PCB to execute -> PID=%u | Estimado=%lu", 
        next_pcb->pid, next_pcb->MT.last_estimated_cpu_burst_ms);

    // Cleanup
    pcb_destroy(pcb1);
    pcb_destroy(pcb2);
    pcb_destroy(pcb3);
    config_destroy(config);

    return 0;
}
