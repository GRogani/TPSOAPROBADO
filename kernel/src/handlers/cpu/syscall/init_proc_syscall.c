#include "init_proc_syscall.h"

void handle_init_proc_syscall(uint32_t caller_pid, uint32_t caller_pc, 
                                     uint32_t new_process_memory_space, 
                                     char *new_process_pseudocode_file,
                                     int response_socket)
{
    LOG_INFO("init_proc_syscall: Procesando syscall INIT_PROC desde PID=%d, creando proceso con memoria=%d bytes, archivo=%s",
             caller_pid, new_process_memory_space, new_process_pseudocode_file ? new_process_pseudocode_file : "NULL");

    // 1. Actualizar PC del proceso actual que está en EXEC
    lock_exec_list();
    t_pcb *current_pcb = find_pcb_in_exec(caller_pid);
    if (current_pcb == NULL)
    {
        LOG_ERROR("init_proc_syscall: Proceso caller con PID %d NO está en EXEC, no se puede actualizar el PC", caller_pid);
        unlock_exec_list();
        send_confirmation_package(response_socket, 1); // 1 = error
        return;
    }
    
    current_pcb->pc = caller_pc;
    unlock_exec_list();

    // 2. Crear PCB del nuevo proceso
    uint32_t new_pid = generate_new_pid();
    t_pcb *new_pcb = pcb_create(new_pid, 0, new_process_memory_space, new_process_pseudocode_file); // PC inicial en 0
    if (new_pcb == NULL)
    {
        LOG_ERROR("init_proc_syscall: Error al crear PCB para nuevo proceso PID %d", new_pid);
        
        // Enviar respuesta de error
        send_confirmation_package(response_socket, 1); // 1 = error
        return;
    }

    // 3. Agregar PCB a la lista NEW
    lock_new_list();
    
    // Verificar si el proceso ya existe antes de agregarlo (aunque no debería pasar con PIDs únicos)
    if (find_pcb_in_new(new_pid))
    {
        unlock_new_list();
        pcb_destroy(new_pcb);
        LOG_ERROR("init_proc_syscall: Proceso con PID %d ya existe en lista NEW", new_pid);
        
        // Enviar respuesta de error
        send_confirmation_package(response_socket, 1); // 1 = error
        return;
    }

    add_pcb_to_new(new_pcb);
    unlock_new_list();

    send_confirmation_package(response_socket, 0); // 0 = success

    LOG_INFO("init_proc_syscall: Respuesta de confirmación enviada a CPU.");

    // 5. Correr largo plazo para intentar inicializar
    LOG_INFO("init_proc_syscall: Ejecutando planificador de largo plazo");
    run_long_scheduler();
    LOG_INFO("init_proc_syscall: Planificador de largo plazo completado");

    // 6. Correr corto plazo
    LOG_INFO("init_proc_syscall: Ejecutando planificador de corto plazo");
    run_short_scheduler();
    LOG_INFO("init_proc_syscall: Syscall INIT_PROC completada para nuevo proceso PID=%d", new_pid);
}

uint32_t generate_new_pid(void)
{    
    // Generar PID basado en tiempo actual y número aleatorio
    // Evitamos PID 0 que está reservado para el proceso root
    uint32_t new_pid;
    do {
        new_pid = (uint32_t)(time(NULL) % 10000) + (rand() % 1000) + 1;
    } while (new_pid == 0); // Asegurar que no sea 0

    return new_pid;
}
