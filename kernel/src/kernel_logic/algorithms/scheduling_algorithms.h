#ifndef KERNEL_SCHEDULING_ALGORITHMS_H
#define KERNEL_SCHEDULING_ALGORITHMS_H

#include "../utils.h"
#include "repository/pcb/pcb.h"
#include "repository/process/new_list.h"
#include "repository/process/ready_list.h"
#include "repository/process/susp_ready_list.h"


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
bool preemption_is_enabled(void);


/**
 * @brief Verifica si se debe desalojar el proceso en ejecucion
 * @param pcb_ready Proceso en READY que se esta considerando para el despacho
 * @param pid_executing Proceso actualmente ejecutándose
 * @return `true` si se debe desalojar, `false` caso contrario
 * @note Utiliza la función `compare_cpu_bursts` para comparar los procesos
 * y la función `preemtion_is_enabled` para verificar si el desalojamiento está habilitado.
 */
bool should_preempt_executing_process(t_pcb *pcb_ready, int32_t pid_executing);

/**
 * @brief Configura los algoritmos de planificación basado en la configuración
 * @note Usa la constante global kernel_config
 */
void configure_scheduling_algorithms();


/**
 * @brief Compara dos procesos por su estimacion de rafaga de CPU
 * @param a Primer proceso a comparar
 * @param b Segundo proceso a comparar
 * @return `true` si el primer proceso tiene menor estimacion que el segundo, false en caso contrario
 */
bool compare_cpu_bursts(void *a, void *b);

bool compare_cpu_bursts_SRT(void *a, void *b);

/**
 * @brief Ordena la lista de READY por Shortest Job First (SJF)
 * @note Utiliza la función `compare_cpu_bursts` para ordenar
 */
void sort_ready_list_by_SJF();

/**
 * @brief Ordena la lista de EXEC por Shortest Job First (SJF)
 * @note Utiliza la función `compare_cpu_bursts` para ordenar
 */
void sort_exec_list_by_SJF();

/**
 * @brief Obtiene el CPU con el proceso de mayor estimacion de CPU
 * @param cpus Lista de CPUs disponibles
 * @note Ordena la lista de Exec y selecciona el primer elemento, con el busca el cpu asociado.
 */
t_cpu_connection *get_cpu_by_SJF(t_list *cpus);

/**
 * @brief Compara dos procesos por su tamaño
 * @param a Primer proceso a comparar
 * @param b Segundo proceso a comparar
 * @return `true` si el primer proceso es más pequeño que el segundo, false en caso contrario
 */
bool compare_process_size(void *a, void *b);


/**
 * @brief Ordena la lista de NEW por Shortest Size First (SSF)
 * @note Utiliza la función `compare_process_size` para ordenar
 */
void sort_new_list_by_SSF();

/**
 * @brief Ordena la lista de SUSP_READY por Shortest Size First (SSF)
 * @note Utiliza la función `compare_process_size` para ordenar
 */
void sort_susp_ready_list_by_SSF();

#endif
