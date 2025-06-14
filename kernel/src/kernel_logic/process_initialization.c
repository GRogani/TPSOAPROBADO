#include "process_initialization.h"

void initialize_root_process(uint32_t pid, uint32_t memory_space, char *pseudocode_file)
{
    LOG_INFO("process_initialization: Inicializando proceso root PID=%d con memoria=%d bytes, archivo=%s",
             pid, memory_space, pseudocode_file ? pseudocode_file : "NULL");

    // 1. Crear nuevo PCB
    t_pcb *new_pcb = pcb_create(pid, 0, memory_space, pseudocode_file); // PC inicial en 0
    if (new_pcb == NULL)
    {
        LOG_ERROR("process_initialization: Error al crear PCB para proceso root PID %d", pid);
        return;
    }

    LOG_INFO("process_initialization: PCB creado exitosamente para proceso root PID=%d", pid);

    // 2. Agregar PCB a la lista NEW (con semáforos)
    lock_new_list();

    // Verificar si el proceso ya existe antes de agregarlo
    if (find_pcb_in_new(pid))
    {
        unlock_new_list();
        pcb_destroy(new_pcb);
        LOG_ERROR("process_initialization: Proceso root con PID %d ya existe en lista NEW", pid);
        return;
    }

    add_pcb_to_new(new_pcb);
    unlock_new_list();

    LOG_INFO("process_initialization: PCB del proceso root agregado a lista NEW para PID=%d", pid);

    // 3. Ejecutar planificador de largo plazo
    LOG_INFO("process_initialization: Ejecutando planificador de largo plazo para proceso root");
    bool success = run_long_scheduler();
    LOG_INFO("process_initialization: Planificador de largo plazo completado");

    // 4. Si se inicializó alguno, correr corto plazo
    if (success)
    {
        LOG_INFO("process_initialization: Ejecutando planificador de corto plazo");
        run_short_scheduler();
        LOG_INFO("process_initialization: Proceso root PID=%d inicializado completamente", pid);
    }
    else
    {
        LOG_WARNING("process_initialization: No se pudo inicializar el proceso root PID=%d", pid);
    }
}
