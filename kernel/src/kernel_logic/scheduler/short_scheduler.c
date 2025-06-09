#include "short_scheduler.h"

t_cpu_connection *get_free_cpu(void)
{
    LOG_INFO("short_scheduler: Buscando CPU libre");

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
        LOG_INFO("short_scheduler: CPU libre encontrada");
    }
    else
    {
        free_cpu = list_get(all_cpus, 0); // Si no hay libres, tomar la primera CPU (fallback)
        LOG_INFO("short_scheduler: No hay CPUs libres disponibles, tomando la primera en la lista de conexiones...");
    }

    list_destroy(all_cpus);

    return free_cpu;
}

cpu_context_package_data send_and_receive_interrupt(int interrupt_socket_id, uint32_t pid)
{
    LOG_INFO("short_scheduler: Enviando interrupción para PID %d", pid);

    // Enviar interrupción
    int sent_bytes = send_interrupt_package(interrupt_socket_id, pid);
    if (sent_bytes <= 0)
    {
        LOG_ERROR("short_scheduler: Error enviando interrupción");
        // return false; ??
    }

    // Esperar confirmación de interrupción (CPU_INTERRUPT response)
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

    t_cpu_connection *cpu = get_free_cpu();

    if (cpu == NULL)
    {
        LOG_ERROR("short_scheduler: No hay CPUs disponibles");
        return;
    }

    lock_cpu(&cpu->cpu_exec_sem);
    lock_ready_list();

    if (list_size(get_ready_list()) == 0)
    {
        // no hay más en ready, vemos si podemos marcar la cpu como libre.
        LOG_INFO("short_scheduler: No hay procesos en READY");
        check_cpu_executing(cpu);
        return;
    }

    bool cpu_is_processing = (cpu->current_process_executing != -1);
    LOG_INFO("short_scheduler: CPU procesando? %s", cpu_is_processing ? "Sí" : "No");
    if (cpu_is_processing)
    {
        // desalojo habilitado?
        bool preemption_enabled = should_preempt_current_process();

        if (!preemption_enabled)
        {
            // do nothing
            LOG_INFO("short_scheduler: CPU ocupada, preemption deshabilitado");
            unlock_cpu(&cpu->cpu_exec_sem);
            return;
        }
        // si preemption está habilitado, go to next
    }

    t_pcb *next_ready = get_next_process_to_dispatch_from_ready();

    if (next_ready == NULL)
    {
        // no deberia ser posible, tiramos error
        LOG_ERROR("short_scheduler: No se encontró ningún proceso en READY, pero la lista no está vacía");
        return;
    }

    // si hay procesos en ready
    LOG_INFO("short_scheduler: Proceso encontrado en READY: PID=%d", next_ready->pid);

    // desalojo enabled?
    bool preemption_enabled = should_preempt_current_process();

    if (preemption_enabled && cpu_is_processing)
    {
        // si se debe desalojar, entonces, usamos la cpu que encontramos arriba
        if (cpu != NULL)
        {
            LOG_INFO("short_scheduler: Realizando preemption del proceso %d", cpu->current_process_executing);

            // send(interrupt) + receive(interrupt)
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
    }
    else
    {
        // lock(EXEC_LIST)
        lock_exec_list();
    }

    // move_process: move next READY to EXEC
    LOG_INFO("short_scheduler: Moviendo proceso PID=%d de READY a EXEC", next_ready->pid);

    // Mover de READY a EXEC
    add_pcb_to_exec(next_ready);

    // send(dispatch, pid, pc)
    send_dispatch_to_cpu(cpu, next_ready->pid, next_ready->pc);

    cpu->current_process_executing = next_ready->pid;

    LOG_INFO("short_scheduler: Proceso PID=%d despachado exitosamente", next_ready->pid);

    // unlock(READY) + unlock(EXEC) + unlock(CPU_SEM)
    unlock_ready_list();
    unlock_exec_list();
    unlock_cpu_connections();
    unlock_cpu(&cpu->cpu_exec_sem);

    LOG_INFO("short_scheduler: Planificador de corto plazo completado");
}

void check_cpu_executing(t_cpu_connection *cpu)
{
    lock_exec_list();

    // cpu is currently processing? (check for existence of current_processing in EXEC list)
    bool process_exists_in_exec = find_pcb_in_exec(cpu->current_process_executing) != NULL;

    if (process_exists_in_exec)
    {
        // si existe en EXEC, no hacer nada, todavia sigue ejecutando y cuando llegue la syscall la vamos a marcar correctamente como no ejecutando.
        LOG_INFO("short_scheduler: Proceso %d sigue en EXEC", cpu->current_process_executing);
        unlock_ready_list();
        unlock_exec_list();
        unlock_cpu(&cpu->cpu_exec_sem);
    }
    else
    {
        // no existe en EXEC (syscall lo movió a otro estado, es el momento de marcarla como no ejecutando)
        LOG_INFO("short_scheduler: Proceso %d ya no está en EXEC, liberando CPU", cpu->current_process_executing);
        cpu->current_process_executing = -1;
        unlock_ready_list();
        unlock_exec_list();
        unlock_cpu(&cpu->cpu_exec_sem);
    }
}