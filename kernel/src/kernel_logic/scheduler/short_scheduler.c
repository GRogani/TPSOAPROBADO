#include "short_scheduler.h"

t_cpu_connection *get_cpu_to_dispatch(void)
{
    LOG_DEBUG("Buscando CPU libre");
    // Buscar CPU que no esté procesando (current_process_executing == -1)
    t_list *all_cpus = (t_list *)get_all_cpu_connections();

    bool cpu_process_not_in_exec(void *ptr)
    {
        t_cpu_connection *cpu = (t_cpu_connection *)ptr;
        if (cpu->current_process_executing == -1)
            return true;

        t_pcb *pcb_in_exec = find_pcb_in_exec(cpu->current_process_executing);
        return pcb_in_exec == NULL; // Retorna true si el proceso NO está en exec (CPU libre)
    };

    t_cpu_connection *available_cpu = list_find(all_cpus, cpu_process_not_in_exec);
    if (available_cpu != NULL)
    {
        LOG_DEBUG("CPU con PID %d no esta en lista EXEC, CPU disponible", available_cpu->current_process_executing);
        available_cpu->current_process_executing = -1;
        list_destroy(all_cpus);
        return available_cpu;
    }

    available_cpu = get_cpu_by_algorithm(all_cpus);
    list_destroy(all_cpus);

    if (available_cpu == NULL)
        LOG_ERROR("No se encontro CPU libre, no se puede despachar");

    return available_cpu;
}

cpu_context_package_data send_and_receive_interrupt(int interrupt_socket_id, int32_t pid)
{
    LOG_INFO("Enviando interrupcion para PID %d", pid);

    int sent_bytes = send_interrupt_package(interrupt_socket_id, pid);
    if (sent_bytes <= 0)
    {
        LOG_ERROR("Error enviando interrupcion");
        // return false; ??
    }

    unlock_exec_list(); // aca permitimos que entren syscalls y otros procesos a la lista EXEC mientras esperamos la respuesta de la interrupcion. podria pasar que el que intentamos desalojar, no esté mas en exec, porque lo procesó una syscall

    t_package *response = recv_package(interrupt_socket_id);
    if (response == NULL)
    {
        LOG_ERROR("Error recibiendo confirmacion de interrupcion");
        // return false; ??
    }

    cpu_context_package_data cpu_context = read_cpu_context_package(response);

    destroy_package(response);

    return cpu_context;
}

bool send_dispatch_to_cpu(t_cpu_connection *cpu_connection, int32_t pid, int32_t pc)
{
    if (cpu_connection == NULL)
    {
        LOG_ERROR("CPU connection es NULL para dispatch");
        return false;
    }

    LOG_INFO("Enviando dispatch para PID %d, PC %d", pid, pc);

    int sent_bytes = send_dispatch_package(cpu_connection->dispatch_socket_id, pid, pc);

    if (sent_bytes <= 0)
    {
        LOG_ERROR("Error enviando dispatch");
        return false;
    }

    LOG_INFO("Dispatch enviado correctamente");
    return true;
}

void get_short_scheduler_context(t_cpu_connection **cpu_out, t_pcb **pcb_ready_out)
{
    *cpu_out = get_cpu_to_dispatch();

    if (*cpu_out == NULL)
    {
        *pcb_ready_out = NULL;
        return;
    }

    *pcb_ready_out = get_next_process_to_dispatch();

    if (*pcb_ready_out == NULL)
    {
        return;
    }
}

void run_short_scheduler(void)
{
    LOG_INFO("Iniciando planificador de corto plazo");

    t_cpu_connection *cpu = NULL;
    t_pcb *pcb_in_ready = NULL;

    lock_cpu_connections();
    LOG_INFO("Lockeado conexiones de CPU");
    lock_ready_list();
    LOG_INFO("Lockeado lista READY");

    get_short_scheduler_context(&cpu, &pcb_in_ready);
    if (cpu == NULL || pcb_in_ready == NULL)
    {
        LOG_INFO("%s", pcb_in_ready ? "No se pudo obtener CPU" : "No se pudo obtener proceso READY");
        unlock_cpu_connections();
        unlock_ready_list();
        return;
    }

    lock_cpu(&cpu->cpu_exec_sem);
    LOG_INFO("Lockeada CPU con ID %d", cpu->id);

    if (cpu->current_process_executing == -1)
    {
        // (4.1) CPU libre: seguimos con el dispatch normalmente
        LOG_INFO("CPU libre para ejecutar proceso con PID %d", pcb_in_ready->pid);
        lock_exec_list();
    }
    else if (!preemption_is_enabled())
    {
        // (4.3) CPU ocupada y no se puede desalojar
        LOG_INFO("CPU ocupada, desalojo no posible");
        unlock_cpu_connections();
        unlock_ready_list();
        unlock_cpu(&cpu->cpu_exec_sem);
        return;
    }
    else if (should_preempt_executing_process(pcb_in_ready, cpu->current_process_executing))
    {
        // (4.2) CPU ocupada, pero se puede desalojar

        cpu_context_package_data cpu_context = send_and_receive_interrupt(cpu->interrupt_socket_id, cpu->current_process_executing);

        if (cpu_context.interrupted_same_pid == 0) // esto significa que el proceso que estaba ejecutando en la cpu es el que mandamos a interrumpir por el pcb_exec, entonces podemos mandar a hacer dispatch
        {
            lock_exec_list(); // como hubo una interrupcion, entonces podemos sacarlo de la lista de exec
            t_pcb *preempted_pcb = remove_pcb_from_exec(cpu->current_process_executing);
            if (preempted_pcb != NULL)
            {
                LOG_OBLIGATORIO("## (%d) - Desalojado por algoritmo SJF/SRT", cpu->current_process_executing);
                preempted_pcb->pc = cpu_context.pc;
                add_pcb_to_ready(preempted_pcb);
                LOG_INFO("Proceso PID=%d movido de EXEC a READY por preemption", preempted_pcb->pid);
            }
            else
            {
                // si no se encuentra, es porque justo lo sacó una syscall, entonces la cpu probablemente quedó libre:
            }
        }
        else
        {
            // aca puede significar que la cpu ya ejecutó una syscall y se autodesalojó, entonces suponemos que la cpu quedó libre y mandamos a reemplazar.
        }
    } else {
        // aca no debe desalojar al proceso que está corriendo actualmente, entonces no hacemos nada
        LOG_INFO("Proceso PID=%d no desalojado, corresponde seguir ejecutando", cpu->current_process_executing);
        unlock_cpu_connections();
        unlock_exec_list();
        unlock_ready_list();
        unlock_cpu(&cpu->cpu_exec_sem);
        return;
    }

    // (5) Realizar dispatch del nuevo proceso
    LOG_INFO("Proceso encontrado en READY con PID %d", pcb_in_ready->pid);

    remove_pcb_from_ready(pcb_in_ready->pid);
    add_pcb_to_exec(pcb_in_ready);
    send_dispatch_to_cpu(cpu, pcb_in_ready->pid, pcb_in_ready->pc);
    cpu->current_process_executing = pcb_in_ready->pid;

    LOG_INFO("Proceso con PID %d despachado correctamente a la cpu: %d", pcb_in_ready->pid, cpu->id);

    // (6) Unlock de recursos
    unlock_cpu_connections();
    unlock_exec_list();
    unlock_ready_list();
    unlock_cpu(&cpu->cpu_exec_sem);

    LOG_INFO("Planificador de corto plazo finalizado");
}
