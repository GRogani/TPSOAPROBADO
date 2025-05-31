#include "scheduling_algorithms.h"
#include "../repository/process/new_list.h"
#include "../repository/process/ready_list.h"
#include "../repository/process/susp_ready_list.h"
#include "utils/logger/logger.h"

// Variables globales para configuración de algoritmos
static PLANIFICATION_ALGORITHM short_term_algorithm = FIFO;
static PLANIFICATION_ALGORITHM long_term_algorithm = FIFO;
static bool preemption_enabled = false;

// ========== ALGORITMOS DE LARGO PLAZO ==========

t_pcb* get_next_process_to_initialize_from_new(void) {
    switch (long_term_algorithm) {
        case FIFO:
            // FIFO: tomar el primer proceso de la lista
            return get_next_pcb_from_new();
            
        case SJF:
            // TODO: Implementar algoritmo de tamaño más pequeño primero (SJF adaptado para largo plazo)
            // Por ahora usar FIFO como fallback
            log_warning(get_logger(), "SJF for long-term not implemented yet, using FIFO");
            return get_next_pcb_from_new();
            
        default:
            log_error(get_logger(), "Unknown long-term algorithm: %d", long_term_algorithm);
            return get_next_pcb_from_new();
    }
}

t_pcb* get_next_process_to_initialize_from_susp_ready(void) {
    switch (long_term_algorithm) {
        case FIFO:
            // FIFO: tomar el primer proceso de la lista
            return get_next_pcb_from_susp_ready();
            
        case SJF:
            // TODO: Implementar algoritmo de tamaño más pequeño primero (SJF adaptado para largo plazo)
            // Por ahora usar FIFO como fallback
            log_warning(get_logger(), "SJF for long-term not implemented yet, using FIFO");
            return get_next_pcb_from_susp_ready();
            
        default:
            log_error(get_logger(), "Unknown long-term algorithm: %d", long_term_algorithm);
            return get_next_pcb_from_susp_ready();
    }
}

// ========== ALGORITMOS DE CORTO PLAZO ==========

t_pcb* get_next_process_to_dispatch_from_ready(void) {
    switch (short_term_algorithm) {
        case FIFO:
            // FIFO: tomar el primer proceso de la lista
            return get_next_pcb_from_ready();
            
        case SJF:
            // TODO: Implementar Shortest Job First
            // Por ahora usar FIFO como fallback
            log_warning(get_logger(), "SJF not implemented yet, using FIFO");
            return get_next_pcb_from_ready();
            
        default:
            log_error(get_logger(), "Unknown short-term algorithm: %d", short_term_algorithm);
            return get_next_pcb_from_ready();
    }
}

bool should_preempt_current_process(void) {
    switch (short_term_algorithm) {
        case FIFO:
            // FIFO sin preemption
            return false;
            
        case SJF:
            // SJF con preemption habilitada si está configurado
            return preemption_enabled;
            
        default:
            return false;
    }
}

// ========== CONFIGURACIÓN DE ALGORITMOS ==========

void configure_scheduling_algorithms(t_kernel_config* config) {
    if (config == NULL) {
        log_error(get_logger(), "Cannot configure algorithms with NULL config");
        return;
    }
    
    // Configurar algoritmo de corto plazo
    short_term_algorithm = config->short_planification_algorithm;
    log_info(get_logger(), "Short-term algorithm configured: %d", short_term_algorithm);
    
    // Configurar algoritmo de largo plazo (usar ready_planification_algorithm como base)
    long_term_algorithm = config->ready_planification_algorithm;
    log_info(get_logger(), "Long-term algorithm configured: %d", long_term_algorithm);
    
    // Configurar preemption (habilitar solo para algoritmos que lo soporten)
    preemption_enabled = (short_term_algorithm == SJF);
    log_info(get_logger(), "Preemption enabled: %s", preemption_enabled ? "true" : "false");
}
