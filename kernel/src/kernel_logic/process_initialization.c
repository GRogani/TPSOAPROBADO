#include "process_initialization.h"

void initialize_root_process(int32_t pid, int32_t memory_space, char *pseudocode_file)
{
    LOG_INFO("Inicializando proceso root con PID %d, memoria %d bytes, archivo %s", pid, memory_space, pseudocode_file ? pseudocode_file : "NULL");

    // 1. Crear nuevo PCB
    t_pcb *new_pcb = pcb_create(pid, 0, memory_space, pseudocode_file); // PC inicial en 0
    if (new_pcb == NULL)
    {
        LOG_ERROR("Error al crear PCB para proceso root con PID %d", pid);
        return;
    }

    LOG_INFO("PCB creado exitosamente para proceso root con PID %d", pid);

    // 2. Agregar PCB a la lista NEW (con semáforos)
    lock_new_list();

    // Verificar si el proceso ya existe antes de agregarlo
    if (find_pcb_in_new(pid))
    {
        unlock_new_list();
        pcb_destroy(new_pcb);
        LOG_ERROR("El proceso root con PID %d ya existe en la lista NEW", pid);
        return;
    }

    add_pcb_to_new(new_pcb);
    unlock_new_list();

    LOG_INFO("PCB del proceso root agregado a la lista NEW para PID %d", pid);

    // 3. Ejecutar planificador de largo plazo
    LOG_INFO("Ejecutando planificador de largo plazo para proceso root");
    bool success = run_long_scheduler();
    LOG_INFO("Planificador de largo plazo finalizado");

    // 4. Si se inicializó alguno, correr corto plazo
    if (success)
    {
        LOG_INFO("Ejecutando planificador de corto plazo");
        run_short_scheduler();
        LOG_INFO("Proceso root con PID %d inicializado completamente", pid);
    }
    else
    {
        LOG_WARNING("No se pudo inicializar el proceso root con PID %d", pid);
    }
}
