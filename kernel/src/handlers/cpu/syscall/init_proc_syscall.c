#include "init_proc_syscall.h"

_Atomic int32_t new_pid = 0; // PID 0 reservado para el proceso root

void handle_init_proc_syscall(int32_t caller_pid, int32_t caller_pc, 
                                     int32_t new_process_memory_space, 
                                     char *new_process_pseudocode_file,
                                     int response_socket)
{
    LOG_INFO("init_proc_syscall: Procesando syscall INIT_PROC desde PID=%d, creando proceso con memoria=%d bytes, archivo=%s",
             caller_pid, new_process_memory_space, new_process_pseudocode_file ? new_process_pseudocode_file : "NULL");

    // 1. Actualizar PC del proceso actual que está en EXEC
    lock_new_list();
    lock_exec_list();

    t_pcb *current_pcb = find_pcb_in_exec(caller_pid);
    if (current_pcb == NULL)
    {
        LOG_ERROR("init_proc_syscall: Proceso caller con PID %d NO está en EXEC, no se puede actualizar el PC", caller_pid);
        unlock_exec_list();
        unlock_new_list();
        return;
    }

    uint64_t time_int_exec = total_time_ms(current_pcb->state_start_time_ms);
    current_pcb->MT.last_cpu_burst_ms = time_int_exec;

    current_pcb->pc = caller_pc;

    // 2. Crear PCB del nuevo proceso
    // int32_t new_pid = generate_new_pid();
    new_pid++;

    t_pcb *new_pcb = pcb_create(new_pid, 0, new_process_memory_space, new_process_pseudocode_file); // PC inicial en 0
    if (new_pcb == NULL)
    {
        LOG_ERROR("init_proc_syscall: Error al crear PCB para nuevo proceso PID %d", new_pid);
        unlock_exec_list();
        unlock_new_list();
        return;
    }
    
    // Verificar si el proceso ya existe antes de agregarlo (aunque no debería pasar con PIDs únicos)
    if (find_pcb_in_new(new_pid))
    {
        unlock_new_list();
        unlock_exec_list();
        pcb_destroy(new_pcb);
        LOG_ERROR("init_proc_syscall: Proceso con PID %d ya existe en lista NEW", new_pid);
        
        return;
    }

    add_pcb_to_new(new_pcb);

    unlock_new_list();
    unlock_exec_list();

    send_confirmation_package(response_socket, true);
    LOG_INFO("init_proc_syscall: Respuesta de confirmación enviada a CPU.");

    // mandamos a un thread asi evitamos que se quede trabada la cpu porque no le vamos a poder contestar desde el kernel si entra otra syscall (porque el corto plazo se queda esperando la respuesta de la interrupcion)
    pthread_t thread;
    if (pthread_create(&thread, NULL, process_schedulers, NULL) != 0)
    {
        LOG_ERROR("INIT_PROC syscall: Failure pthread_create");
    }
    pthread_detach(thread);
}

void process_schedulers() {
    // 5. Correr largo plazo para intentar inicializar
    LOG_INFO("init_proc_syscall: Ejecutando planificador de largo plazo");
    run_long_scheduler();
    LOG_INFO("init_proc_syscall: Planificador de largo plazo completado");

    // 6. Correr corto plazo
    LOG_INFO("init_proc_syscall: Ejecutando planificador de corto plazo");
    run_short_scheduler();
    LOG_INFO("init_proc_syscall: Syscall INIT_PROC completada para nuevo proceso PID=%d", new_pid);
}

int32_t generate_new_pid(void)
{    
    // Generar PID basado en tiempo actual y número aleatorio
    // Evitamos PID 0 que está reservado para el proceso root
    int32_t new_pid;
    do {
        new_pid = (int32_t)(time(NULL) % 10000) + (rand() % 1000) + 1;
    } while (new_pid == 0); // Asegurar que no sea 0

    return new_pid;
}
