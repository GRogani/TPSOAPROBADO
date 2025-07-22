#ifndef KERNEL_SHORT_SCHEDULER_H
#define KERNEL_SHORT_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "../utils.h"
#include "enums/Eopcodes.h"
#include "collections/collections.h"
#include "kernel_logic/algorithms/scheduling_algorithms.h"
#include "repository/process/exec_list.h"

/**
 * @brief Ejecuta el planificador de corto plazo
 * Implementa el algoritmo de scheduling con manejo de preemption y CPU assignment
 */
void run_short_scheduler(void);


/**
 * @brief Obtiene el contexto necesario para el planificador de corto plazo
 * @param cpu_out Puntero a CPU connection disponible
 * @param pcb_ready_out Puntero al PCB en READY
 * @param pcb_exec_out Puntero al PCB en EXEC
 */
void get_short_scheduler_context(t_cpu_connection** cpu_out, t_pcb** pcb_ready_out);

/**
 * @brief Obtiene una CPU libre del pool de conexiones
 * @return Puntero a CPU connection disponible o NULL si no hay disponibles
 */
t_cpu_connection *get_cpu_to_dispatch(void);

/**
 * @brief Envía interrupción a CPU y espera confirmación
 * @param cpu_connection CPU a la que enviar la interrupción
 * @param pid PID del proceso a interrumpir
 * @return true si la interrupción fue exitosa, false en caso contrario
 */
cpu_context_package_data send_and_receive_interrupt(int interrupt_socket_id, int32_t pid);

/**
 * @brief Envía dispatch (DISPATCH) a CPU
 * @param cpu_connection CPU a la que enviar el dispatch
 * @param pid PID del proceso
 * @param pc Program Counter del proceso
 * @return true si el dispatch fue exitoso, false en caso contrario
 */
bool send_dispatch_to_cpu(t_cpu_connection *cpu_connection, int32_t pid, int32_t pc);

void check_cpu_executing(t_cpu_connection *cpu);

#endif