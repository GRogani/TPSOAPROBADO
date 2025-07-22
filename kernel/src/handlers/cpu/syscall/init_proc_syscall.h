#ifndef KERNEL_INIT_PROC_SYSCALL_H
#define KERNEL_INIT_PROC_SYSCALL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "../utils.h"
#include "repository/pcb/pcb.h"
#include "repository/process/new_list.h"
#include "repository/process/exec_list.h"
#include "kernel_logic/scheduler/long_scheduler.h"
#include "kernel_logic/scheduler/short_scheduler.h"
#include "utils/DTPs/confirmation_package.h"


/**
 * @brief Maneja la syscall de inicialización de procesos (cuando llega desde CPU)
 * Esta función es llamada desde handle_dispatch_client.c cuando se recibe una syscall INIT_PROC
 * 
 * Flujo:
 * 1. Actualizar PC del proceso actual que está en EXEC
 * 2. Crear PCB del nuevo proceso
 * 3. Agregarlo a la lista NEW
 * 4. Mandar respuesta de que se agregó
 * 5. Correr largo plazo para intentar inicializar
 * 6. Correr corto plazo
 * 
 * @param caller_pid PID del proceso que llama a la syscall (proceso actual en EXEC)
 * @param caller_pc PC del proceso que llama a la syscall
 * @param new_process_memory_space Espacio en memoria para el nuevo proceso
 * @param new_process_pseudocode_file Archivo de pseudocódigo del nuevo proceso
 * @param response_socket Socket para enviar la respuesta de confirmación
 */
void handle_init_proc_syscall(int32_t caller_pid, int32_t caller_pc, 
                                     int32_t new_process_memory_space, 
                                     char *new_process_pseudocode_file,
                                     int response_socket);

/**
 * @brief Genera un nuevo PID único para el proceso a crear
 * @return PID único generado
 */
int32_t generate_new_pid(void);

void process_schedulers();

#endif
