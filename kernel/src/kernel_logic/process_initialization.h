#ifndef KERNEL_PROCESS_INITIALIZATION_H
#define KERNEL_PROCESS_INITIALIZATION_H

#include <stdint.h>
#include <stdbool.h>
#include "../utils.h"
#include "repository/pcb/pcb.h"
#include "repository/process/new_list.h"
#include "scheduler/long_scheduler.h"
#include "scheduler/short_scheduler.h"

/**
 * @brief Inicializa el proceso root (PID 0) del sistema
 * Esta función es llamada únicamente desde main.c para crear el primer proceso
 * 
 * Flujo:
 * 1. Crear PCB
 * 2. Agregarlo a la lista NEW
 * 3. Correr largo plazo
 * 4. Si se inicializó alguno, correr corto plazo
 * 
 * @param pid Process ID (normalmente 0 para el proceso root)
 * @param memory_space Espacio en memoria asignado al proceso
 * @param pseudocode_file Nombre del archivo de pseudocódigo del proceso
 */
void initialize_root_process(int32_t pid, int32_t memory_space, char *pseudocode_file);

#endif
