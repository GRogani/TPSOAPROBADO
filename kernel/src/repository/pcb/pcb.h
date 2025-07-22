#ifndef KERNEL_PCB_H
#define KERNEL_PCB_H

#include <stdint.h>
#include <sys/time.h>
#include <enums/Eprocess_state.h>
#include <utils/safe_alloc.h>
#include <stdlib.h>
#include <string.h>
#include "../utils.h"


#define PROCESS_STATES_COUNT 7

/**
 * @brief Estructura para métricas de estado (ME) - cuenta las veces que el proceso estuvo en cada estado
 */
typedef struct {
    int32_t new_count;
    int32_t ready_count;
    int32_t exec_count;
    int32_t blocked_count;
    int32_t susp_blocked_count;
    int32_t susp_ready_count;
    int32_t exit_count;
} t_state_metrics;

/**
 * @brief Estructura para métricas de tiempo (MT) - tiempo en milisegundos en cada estado
 */
typedef struct {
    uint64_t last_estimated_cpu_burst_ms;
    uint64_t last_cpu_burst_ms;          
    uint64_t new_time_ms;
    uint64_t ready_time_ms;
    uint64_t exec_time_ms;
    uint64_t blocked_time_ms;
    uint64_t susp_blocked_time_ms;
    uint64_t susp_ready_time_ms;
    uint64_t exit_time_ms;
} t_time_metrics;

/**
 * @brief Process Control Block (PCB)
 */
typedef struct t_pcb {
    int32_t pc;                    // Program Counter
    int32_t pid;                   // Process ID
    int32_t size;                  // Tamaño del proceso en memoria
    char* pseudocode_file;          // Nombre del archivo de pseudocódigo
    PROCESS_STATE current_state;    // Estado actual del proceso
    uint64_t state_start_time_ms; // Tiempo de inicio del estado actual
    t_state_metrics ME;             // Métricas de Estado
    t_time_metrics MT;              // Métricas de Tiempo
} t_pcb;

/**
 * @brief Crea un nuevo PCB inicializado.
 * @param pid Process ID del nuevo proceso.
 * @param pc Program Counter inicial.
 * @param size Tamaño del proceso en memoria.
 * @param pseudocode_file Nombre del archivo de pseudocódigo del proceso.
 * @return Puntero al PCB creado o NULL si falla.
 */
t_pcb* pcb_create(int32_t pid, int32_t pc, int32_t size, char* pseudocode_file);

/**
 * @brief Libera la memoria de un PCB.
 * @param pcb Puntero al PCB a liberar.
 */
void pcb_destroy(t_pcb* pcb);

/**
 * @brief Obtiene el nombre del archivo de pseudocódigo de un PCB.
 * @param pcb Puntero al PCB.
 * @return Puntero al nombre del archivo o NULL si no está definido.
 */
char* pcb_get_pseudocode_file(t_pcb* pcb);

/**
 * @brief Cambia el estado de un proceso y actualiza las métricas.
 * @param pcb Puntero al PCB.
 * @param new_state Nuevo estado del proceso.
 */
void pcb_change_state(t_pcb* pcb, PROCESS_STATE new_state);

/**
 * @brief Obtiene el tiempo actual en milisegundos.
 * @return Tiempo actual en milisegundos.
 */
uint64_t get_current_time_ms(void);

/**
 * @brief Calcula la diferencia de tiempo en milisegundos entre dos timestamps.
 * @param start Tiempo de inicio.
 * @param end Tiempo de fin.
 * @return Diferencia en milisegundos.
 */
//uint64_t time_diff_ms(struct timeval start, struct timeval end);

/**
 * @return Devuelve el tiempo total restando el dado al actual.
 */
uint64_t total_time_ms(u_int64_t start_time_ms);

#endif
