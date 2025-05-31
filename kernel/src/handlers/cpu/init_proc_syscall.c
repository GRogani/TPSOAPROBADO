#include "init_proc_syscall.h"

/**
 * @brief Syscall for process initialization.
 * Creates a new PCB, adds it to NEW list, runs long-term scheduler, and optionally short-term scheduler.
 * @param args Structure containing pid and memory_space (t_init_proc_args*)
 */
void init_proc(void *args)
{
    if (args == NULL) {
        LOG_ERROR("init_proc: argumentos nulos recibidos");
        return;
    }

    // 1. Extraer argumentos
    t_init_proc_args *proc_args = (t_init_proc_args*)args;
    uint32_t pid = proc_args->pid;
    uint32_t memory_space = proc_args->memory_space;
    char* pseudocode_file = proc_args->pseudocode_file;
    
    LOG_INFO("init_proc: Inicializando proceso PID=%d con memoria=%d bytes, archivo=%s\n", 
             pid, memory_space, pseudocode_file ? pseudocode_file : "NULL");

    // 2. Crear nuevo PCB con archivo de pseudocódigo y tamaño
    t_pcb* new_pcb = pcb_create(pid, 0, memory_space, pseudocode_file); // PC inicial en 0
    if (new_pcb == NULL) {
        LOG_ERROR("init_proc: Error al crear PCB para PID %d", pid);
        return;
    }

    LOG_INFO("init_proc: PCB creado exitosamente para PID=%d\n", pid);

    // 3. Agregar PCB a la lista NEW (con semáforos)
    lock_new_list();
    
    // Verificar si el proceso ya existe antes de agregarlo
    if (find_pcb_in_new(pid)) {
        unlock_new_list();
        pcb_destroy(new_pcb);
        LOG_ERROR("init_proc: Proceso con PID %d ya existe en lista NEW", pid);
        return;
    }
    
    add_pcb_to_new(new_pcb);
    unlock_new_list();
    
    LOG_INFO("init_proc: PCB agregado a lista NEW para PID=%d\n", pid);

    // 4. Ejecutar planificador de largo plazo
    LOG_INFO("init_proc: Planificador de largo plazo iniciado\n");
    bool success = run_long_scheduler();
    LOG_INFO("init_proc: Planificador de largo plazo completado\n");

    if(success) {
        LOG_INFO("init_proc: Ejecutando planificador de corto plazo\n");
        run_short_scheduler();
        LOG_INFO("init_proc: Proceso PID=%d inicializado completamente\n", pid);
    }
}

/**
 * @brief Función auxiliar para manejar la syscall init_proc desde handlers externos
 * @param pid Process ID del nuevo proceso
 * @param memory_space Espacio en memoria asignado al proceso
 * @param pseudocode_file Nombre del archivo de pseudocódigo del proceso
 */
void handle_init_proc_syscall(uint32_t pid, uint32_t memory_space, char* pseudocode_file)
{
    // Crear estructura de argumentos
    t_init_proc_args args = {
        .pid = pid,
        .memory_space = memory_space,
        .pseudocode_file = NULL  // Se asignará abajo si no es NULL
    };
    
    // Copiar el nombre del archivo si se proporciona
    if (pseudocode_file != NULL) {
        size_t filename_len = strlen(pseudocode_file) + 1;
        args.pseudocode_file = malloc(filename_len);
        if (args.pseudocode_file != NULL) {
            strcpy(args.pseudocode_file, pseudocode_file);
        }
    }
    
    // Llamar a la syscall
    init_proc(&args);
    
    // Liberar memoria temporal si fue asignada
    if (args.pseudocode_file != NULL) {
        free(args.pseudocode_file);
    }
}