#ifndef KERNEL_SHORT_SCHEDULER_H
#define KERNEL_SHORT_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "../utils.h"
#include "enums/Eopcodes.h"
#include "../../collections/collections.h"
#include "../../algorithms/scheduling_algorithms.h"

/**
 * @brief Ejecuta el planificador de corto plazo
 * Implementa el algoritmo de scheduling con manejo de preemption y CPU assignment
 */
void run_short_scheduler(void);

/**
 * @brief Obtiene una CPU libre del pool de conexiones
 * @return Puntero a CPU connection disponible o NULL si no hay disponibles
 */
t_cpu_connection *get_free_cpu(void);

/**
 * @brief Envía interrupción a CPU y espera confirmación
 * @param cpu_connection CPU a la que enviar la interrupción
 * @param pid PID del proceso a interrumpir
 * @return true si la interrupción fue exitosa, false en caso contrario
 */
t_cpu_interrupt *send_and_receive_interrupt(int interrupt_socket_id, uint32_t pid);

/**
 * @brief Envía dispatch (PID_PC_PACKAGE) a CPU
 * @param cpu_connection CPU a la que enviar el dispatch
 * @param pid PID del proceso
 * @param pc Program Counter del proceso
 * @return true si el dispatch fue exitoso, false en caso contrario
 */
bool send_dispatch_to_cpu(t_cpu_connection *cpu_connection, uint32_t pid, uint32_t pc);

#endif