#ifndef KERNEL_SCHEDULING_ALGORITHMS_H
#define KERNEL_SCHEDULING_ALGORITHMS_H

#include "utils/config/t_configs.h"
#include "repository/pcb/pcb.h"
#include "repository/process/new_list.h"
#include "repository/process/ready_list.h"
#include "repository/process/susp_ready_list.h"
#include "../utils.h"

/**
 * @brief Funciones de abstracción para algoritmos de planificación
 * 
 * Estas funciones abstraen la lógica de selección de procesos de los schedulers,
 * permitiendo cambiar algoritmos sin modificar la implementación del scheduler.
 */

// ========== ALGORITMOS DE LARGO PLAZO ==========

/**
 * @brief Obtiene el siguiente proceso a inicializar de la lista NEW
 * @return PCB del proceso seleccionado o NULL si no hay procesos
 * @note Inicialmente usa FIFO, pero puede cambiarse por "smallest size first"
 */
t_pcb* get_next_process_to_initialize_from_new(void);

/**
 * @brief Obtiene el siguiente proceso a des-suspender de la lista SUSP_READY
 * @return PCB del proceso seleccionado o NULL si no hay procesos
 * @note Inicialmente usa FIFO, pero puede cambiarse por "smallest size first"
 */
t_pcb* get_next_process_to_initialize_from_susp_ready(void);

// ========== ALGORITMOS DE CORTO PLAZO ==========

/**
 * @brief Obtiene el siguiente proceso a despachar. Sin desalojo solo chequea la lista READY.
 * @return PCB del proceso seleccionado o NULL si no hay procesos
 * @note Inicialmente usa FIFO, pero puede cambiarse por SJF con/sin preemption
 */
t_pcb* get_next_process_to_dispatch(void);

t_cpu_connection* get_cpu_by_algorithm(t_list *cpus);

/**
 * @brief Verifica si se debe realizar preemption en el scheduler de corto plazo
 * @return true si se debe hacer preemption, false caso contrario
 * @note Inicialmente false (FIFO), pero se puede habilitar para SJF con preemption
 */
bool should_preempt_current_process(void);

// ========== CONFIGURACIÓN DE ALGORITMOS ==========

/**
 * @brief Configura los algoritmos de planificación basado en la configuración
 * @param config Configuración del kernel
 */
void configure_scheduling_algorithms(t_kernel_config* config);

// SJF

bool compare_cpu_bursts(void *a, void *b);

void sort_ready_list_by_SJF();

void sort_exec_list_by_SJF();

void sort_susp_ready_list_by_SJF();

void sort_cpu_list_by_SJF(t_list *cpus);

#endif
