#ifndef KERNEL_INIT_PROC_SYSCALL_H
#define KERNEL_INIT_PROC_SYSCALL_H

#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "../../collections/collections.h"
#include "../../repository/pcb.h"
#include "../../repository/process/new_list.h"
#include "../utils.h"
#include "../scheduler/long_scheduler.h"
#include "../scheduler/short_scheduler.h"

/**
 * @brief Estructura para los argumentos de init_proc
 */
typedef struct {
    uint32_t pid;           // Process ID
    uint32_t memory_space;  // Espacio en memoria asignado
    char* pseudocode_file;  // Nombre del archivo de pseudocódigo
} t_init_proc_args;

/**
 * @brief Syscall para inicialización de procesos.
 * Crea un nuevo PCB, lo agrega a la lista NEW, ejecuta planificador de largo plazo y corto plazo.
 * @param args Estructura t_init_proc_args con pid y memory_space
 */
void init_proc(void *args);

/**
 * @brief Función auxiliar para manejar la syscall init_proc desde handlers externos
 * @param pid Process ID del nuevo proceso
 * @param memory_space Espacio en memoria asignado al proceso
 * @param pseudocode_file Nombre del archivo de pseudocódigo del proceso
 */
void handle_init_proc_syscall(uint32_t pid, uint32_t memory_space, char* pseudocode_file);

#endif