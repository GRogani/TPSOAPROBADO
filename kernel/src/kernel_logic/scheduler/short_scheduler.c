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
    };

    t_cpu_connection *free_cpu = list_find(all_cpus, is_cpu_free);
    if (free_cpu != NULL)
    {
        LOG_DEBUG("short_scheduler: CPU libre encontrada");
        list_destroy(all_cpus);
        return free_cpu;
    }

    bool cpu_process_not_in_exec(void *ptr)
    {
        t_cpu_connection *cpu = (t_cpu_connection *)ptr;
        if (cpu->current_process_executing == -1)
            return true;
        
        t_pcb *pcb_in_exec = find_pcb_in_exec(cpu->current_process_executing);
        return pcb_in_exec == NULL; // Retorna true si el proceso NO está en exec (CPU libre)
    };

    t_cpu_connection *available_cpu = list_find(all_cpus, cpu_process_not_in_exec);
    if (available_cpu)
    {
        LOG_DEBUG("short_scheduler: CPU con PID %d no está en lista EXEC, CPU disponible", 
                   available_cpu->current_process_executing);
        available_cpu->current_process_executing = -1;
        list_destroy(all_cpus);
        return available_cpu;
    }

    free_cpu =  get_cpu_by_algorithm(all_cpus);
    list_destroy(all_cpus);

    if (free_cpu == NULL)
        LOG_ERROR("short_scheduler: No se encontró CPU libre, no se puede despachar");

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

void get_short_scheduler_context(t_cpu_connection** cpu_out, t_pcb** pcb_ready_out, t_pcb** pcb_exec_out)
{
    *cpu_out = get_cpu_to_dispatch();

    if (*cpu_out == NULL) {
        *pcb_ready_out = NULL;
        *pcb_exec_out = NULL;
        return;
    }

    *pcb_ready_out = get_next_process_to_dispatch();

    if (*pcb_ready_out == NULL) {
        *pcb_exec_out = NULL;
        return;
    }

    *pcb_exec_out = find_pcb_in_exec((*cpu_out)->current_process_executing);
}


void run_short_scheduler(void)
{
    LOG_INFO("short_scheduler: Iniciando planificador de corto plazo");

    t_cpu_connection* cpu = NULL;
    t_pcb* pcb_in_ready = NULL;
    t_pcb* pcb_in_exec = NULL;

    lock_cpu_connections();
    lock_ready_list();
    lock_exec_list();

    get_short_scheduler_context(&cpu, &pcb_in_ready, &pcb_in_exec);
    if (cpu == NULL || pcb_in_ready == NULL) {
        LOG_INFO("short_scheduler: %s", pcb_in_ready ? "No se pudo obtener CPU" : "No se pudo obtener proceso READY");
        unlock_cpu_connections();
        unlock_ready_list();
        unlock_exec_list();
        return;
    }

    lock_cpu(&cpu->cpu_exec_sem);

    if (cpu->current_process_executing == -1)
    {
        // (4.1) CPU libre: seguimos con el dispatch normalmente
        LOG_INFO("short_scheduler: CPU libre para ejecutar proceso PID=%d", pcb_in_ready->pid);
    }
    else if (should_preempt_executing_process(pcb_in_ready, pcb_in_exec))
    {
        // (4.2) CPU ocupada, pero se puede desalojar
        LOG_OBLIGATORIO("## (%d) - Desalojado por algoritmo SJF/SRT", cpu->current_process_executing);

        cpu_context_package_data cpu_context = send_and_receive_interrupt(cpu->interrupt_socket_id, cpu->current_process_executing);

        if (cpu_context.interrupted_same_pid == 0)
        {
            t_pcb *preempted_pcb = remove_pcb_from_exec(cpu->current_process_executing);
            if (preempted_pcb != NULL)
            {
                preempted_pcb->pc = cpu_context.pc;
                add_pcb_to_ready(preempted_pcb);
                LOG_INFO("short_scheduler: Proceso PID=%d movido de EXEC a READY por preemption", preempted_pcb->pid);
            }
            cpu->current_process_executing = -1;
        }
    }
    else
    {
        // (4.3) CPU ocupada y no se puede desalojar
        LOG_INFO("short_scheduler: CPU ocupada, desalojo no posible");
        unlock_cpu_connections();
        unlock_exec_list();
        unlock_ready_list();
        unlock_cpu(&cpu->cpu_exec_sem);
        return;
    }

    // (5) Realizar dispatch del nuevo proceso
    LOG_INFO("short_scheduler: Proceso encontrado en READY: PID=%d", pcb_in_ready->pid);

    remove_pcb_from_ready(pcb_in_ready->pid);
    add_pcb_to_exec(pcb_in_ready);
    send_dispatch_to_cpu(cpu, pcb_in_ready->pid, pcb_in_ready->pc);
    cpu->current_process_executing = pcb_in_ready->pid;

    LOG_INFO("short_scheduler: Proceso PID=%d despachado exitosamente", pcb_in_ready->pid);

    // (6) Unlock de recursos
    unlock_cpu_connections();
    unlock_exec_list();
    unlock_ready_list();
    unlock_cpu(&cpu->cpu_exec_sem);

    LOG_INFO("short_scheduler: Planificador de corto plazo completado");
}
