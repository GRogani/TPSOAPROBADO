#include "scheduling_algorithms.h"


// Variables globales para configuración de algoritmos
static PLANIFICATION_ALGORITHM short_term_algorithm;
static PLANIFICATION_ALGORITHM long_term_algorithm;
static bool preemption_enabled = false;

// ========== ALGORITMOS DE LARGO PLAZO ==========

t_pcb* get_next_process_to_initialize_from_new(void) 
{
    if (long_term_algorithm == SJF)
    {
            LOG_WARNING("SJF for long-term not implemented yet, using FIFO");
            // TODO : Implementar SJF sort para new
    }

    return get_next_pcb_from_new();
    
}

t_pcb* get_next_process_to_initialize_from_susp_ready(void) 
{
    if (long_term_algorithm == SJF)
    {
        LOG_WARNING("SJF for long-term not implemented yet, using FIFO");
        // TODO : Implementar SJF sort para susp_ready
    }

    return get_next_pcb_from_susp_ready();
}

// ========== ALGORITMOS DE CORTO PLAZO ==========

t_cpu_connection get_cpu_by_algorithm(t_list *cpus)
{
    if (short_term_algorithm == SJF)
        sort_cpu_list_by_SJF (cpus);

    return = list_get(cpus, 0);
}

t_pcb* get_next_process_to_dispatch(void) 
{
    if (short_term_algorithm == SJF)     
    {
        sort_ready_list_by_SJF();
        return get_next_pcb_from_ready();
    }
    else // FIFO
        return get_next_pcb_from_ready();
    
}

bool should_preempt_current_process(void) 
{
    if (short_term_algorithm == SJF)
        return preemption_enabled;

    else // FIFO
        return false;

}

// ========== CONFIGURACIÓN DE ALGORITMOS ==========

void configure_scheduling_algorithms(t_kernel_config* config) 
{
    if (config == NULL) {
        LOG_ERROR("Cannot configure algorithms with NULL config");
        return;
    }
    
    // Configurar algoritmo de corto plazo
    if (config->short_planification_algorithm == SJF)
        short_term_algorithm = config->short_planification_algorithm;
    else
        short_term_algorithm = FIFO; // Fallback a FIFO en cualquier otro caso

    LOG_INFO("Short-term algorithm configured: %d", short_term_algorithm);
    
    // Configurar algoritmo de largo plazo (usar ready_planification_algorithm como base)
    long_term_algorithm = config->ready_planification_algorithm;
    LOG_INFO("Long-term algorithm configured: %d", long_term_algorithm);
    
    // Configurar preemption (habilitar solo para algoritmos que lo soporten)
    preemption_enabled = (short_term_algorithm == SJF);
    LOG_INFO("Preemption enabled: %s", preemption_enabled ? "true" : "false");
}
