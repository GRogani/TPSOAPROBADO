#include "short_scheduler.h"

t_cpu_connection *get_cpu_to_dispatch(void)
{
    LOG_DEBUG("short_scheduler: Buscando CPU libre");

    // Buscar CPU que no esté procesando (current_process_executing == -1)
    t_list *all_cpus = (t_list *)get_all_cpu_connections();

    bool is_cpu_free(void *ptr)
    {
        t_cpu_connection *cpu = (t_cpu_connection *)ptr;
        return cpu->current_process_executing == -1;
    }

    t_cpu_connection *free_cpu = list_find(all_cpus, is_cpu_free);

    if (free_cpu != NULL)
    {
        LOG_DEBUG("short_scheduler: CPU libre encontrada");
    }
    
    free_cpu =  get_cpu_by_algorithm(all_cpus);

    list_destroy(all_cpus);

    return free_cpu;
}

cpu_context_package_data send_and_receive_interrupt(int interrupt_socket_id, uint32_t pid)
{
    LOG_INFO("short_scheduler: Enviando interrupción para PID %d", pid);

    int sent_bytes = send_interrupt_package(interrupt_socket_id, pid);
    if (sent_bytes <= 0)
    {
        LOG_ERROR("short_scheduler: Error enviando interrupción");
        // return false; ??
    }

    t_package *response = recv_package(interrupt_socket_id);
    if (response == NULL)
    {
        LOG_ERROR("short_scheduler: Error recibiendo confirmación de interrupción");
        // return false; ??
    }

    cpu_context_package_data cpu_context = read_cpu_context_package(response);

    destroy_package(response);

    return cpu_context;
}

bool send_dispatch_to_cpu(t_cpu_connection *cpu_connection, uint32_t pid, uint32_t pc)
{
    if (cpu_connection == NULL)
    {
        LOG_ERROR("short_scheduler: CPU connection es NULL para dispatch");
        return false;
    }

    LOG_INFO("short_scheduler: Enviando dispatch PID=%d, PC=%d", pid, pc);

    int sent_bytes = send_dispatch_package(cpu_connection->dispatch_socket_id, pid, pc);

    if (sent_bytes <= 0)
    {
        LOG_ERROR("short_scheduler: Error enviando dispatch");
        return false;
    }

    LOG_INFO("short_scheduler: Dispatch enviado exitosamente");
    return true;
}

void run_short_scheduler(void)
{
    LOG_INFO("short_scheduler: Iniciando planificador de corto plazo");

    t_cpu_connection *cpu = get_cpu_to_dispatch();

    if (cpu == NULL)
    {
        LOG_ERROR("short_scheduler: No se encontro CPU para dispatch");
        return;
    }

    lock_cpu(&cpu->cpu_exec_sem);

    lock_ready_list();

    lock_exec_list();
        t_pcb *pcb_in_exec = find_pcb_in_exec(cpu->current_process_executing);
        bool cpu_executing = (pcb_in_exec != NULL);
        if (!cpu_executing)
            cpu->current_process_executing = -1;
    unlock_exec_list();

    bool cpu_is_processing = (cpu->current_process_executing != -1);

    LOG_INFO("short_scheduler: CPU procesando? %s", cpu_is_processing ? "Sí" : "No");

    if (cpu_is_processing && !preepmtion_is_enabled())
    {
        LOG_INFO("short_scheduler: CPU ocupada, desalojo deshabilitado");
        unlock_cpu(&cpu->cpu_exec_sem);
        unlock_ready_list();
        return;
    }

    t_pcb *pcb_in_ready = get_next_process_to_dispatch();

    if (pcb_in_ready == NULL)
    {
        LOG_INFO("short_scheduler: No se encontró ningún proceso en READY");
        unlock_cpu(&cpu->cpu_exec_sem);
        unlock_ready_list();
        return;
    }

    LOG_INFO("short_scheduler: Proceso encontrado en READY: PID=%d", pcb_in_ready->pid);

    if ( should_preempt_executing_process(pcb_in_ready, pcb_in_exec) )
    {
        LOG_INFO("short_scheduler: Desalojando el proceso %d", cpu->current_process_executing);

        cpu_context_package_data cpu_context = send_and_receive_interrupt(cpu->interrupt_socket_id, cpu->current_process_executing);

        lock_exec_list();

        if (cpu_context.interrupted_same_pid == 0)
        { // 0 = success
            // move current_processing from EXEC to READY
            t_pcb *preempted_pcb = remove_pcb_from_exec(cpu->current_process_executing);
            if (preempted_pcb != NULL)
            {
                preempted_pcb->pc = cpu_context.pc; // Actualizar PC del PCB preemptado
                add_pcb_to_ready(preempted_pcb);
                LOG_INFO("short_scheduler: Proceso %d movido de EXEC a READY por preemption", cpu->current_process_executing);
            }

            // Liberar la CPU anterior
            cpu->current_process_executing = -1;
        }

        // no se interrumpió el proceso porque no estaba ejecutando nada. entonces no debemos hacer nada con el proceso ejecutando.
        // simplemente ignoramos y pasamos el nuevo proceso de READY a EXEC.
        
    }
    else
    {
        lock_exec_list();
    }

    LOG_INFO("short_scheduler: Moviendo proceso PID=%d de READY a EXEC", pcb_in_ready->pid);

    add_pcb_to_exec(pcb_in_ready);

    send_dispatch_to_cpu(cpu, pcb_in_ready->pid, pcb_in_ready->pc);

    cpu->current_process_executing = pcb_in_ready->pid;

    LOG_INFO("short_scheduler: Proceso PID=%d despachado exitosamente", pcb_in_ready->pid);

    unlock_ready_list();
    unlock_exec_list();
    unlock_cpu_connections();
    unlock_cpu(&cpu->cpu_exec_sem);

    LOG_INFO("short_scheduler: Planificador de corto plazo completado");
}