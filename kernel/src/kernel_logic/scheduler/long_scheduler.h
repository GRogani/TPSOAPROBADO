#ifndef KERNEL_LONG_SCHEDULER_H
#define KERNEL_LONG_SCHEDULER_H

#include <stddef.h>
#include "../utils.h"
#include "kernel_logic/algorithms/scheduling_algorithms.h"
#include "handlers/memory/client/memory_client.h"
#include "repository/process/new_list.h"
#include "repository/process/ready_list.h"
#include "repository/process/susp_ready_list.h"
#include "repository/pcb/pcb.h"



/**
 * @brief Ejecuta el planificador de largo plazo
 * 
 * Implementa el algoritmo de planificación de largo plazo que:
 * 1. Procesa procesos suspendidos listos (SUSP_READY -> READY)
 * 2. Procesa procesos nuevos (NEW -> READY)
 * 3. Se comunica con memoria para inicializar/des-suspender procesos
 * 4. Utiliza algoritmos abstractos para selección de procesos
 * 
 * @return NULL (por compatibilidad con pthread)
 */
bool run_long_scheduler(void);

#endif
