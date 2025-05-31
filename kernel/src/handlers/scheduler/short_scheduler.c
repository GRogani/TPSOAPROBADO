#include "short_scheduler.h"

t_cpu_connection* get_free_cpu(void) {
    log_debug(get_logger(), "short_scheduler: Buscando CPU libre");
    
    // Buscar CPU que no esté procesando (current_process_executing == -1)
    t_list *all_cpus = (t_list *) get_all_cpu_connections();

    bool is_cpu_free(void* ptr) {
        t_cpu_connection* cpu = (t_cpu_connection*)ptr;
        return cpu->current_process_executing == -1;
    }
    
    t_cpu_connection* free_cpu = list_find(all_cpus, is_cpu_free);
    list_destroy(all_cpus);
    
    if (free_cpu != NULL) {
        log_debug(get_logger(), "short_scheduler: CPU libre encontrada");
    } else {
        free_cpu = list_get(all_cpus, 0); // Si no hay libres, tomar la primera CPU (fallback)
        log_debug(get_logger(), "short_scheduler: No hay CPUs libres disponibles");
    }
    
    return free_cpu;
}

t_cpu_interrupt *send_and_receive_interrupt(int interrupt_socket_id, uint32_t pid)
{
    log_info(get_logger(), "short_scheduler: Enviando interrupción para PID %d", pid);
    
    // Enviar interrupción
    int sent_bytes = send_cpu_interrupt_request(interrupt_socket_id, pid);
    if (sent_bytes <= 0) {
        log_error(get_logger(), "short_scheduler: Error enviando interrupción");
        return false;
    }
    
    // Esperar confirmación de interrupción (CPU_INTERRUPT response)
    t_package* response = recv_package(interrupt_socket_id);
    if (response == NULL) {
        log_error(get_logger(), "short_scheduler: Error recibiendo confirmación de interrupción");
        return false;
    }
    t_cpu_interrupt * interrupt_info = read_cpu_interrupt_response(response);
    
    package_destroy(response);
    
    return interrupt_info;
}

bool send_dispatch_to_cpu(t_cpu_connection* cpu_connection, uint32_t pid, uint32_t pc) {
    if (cpu_connection == NULL) {
        log_error(get_logger(), "short_scheduler: CPU connection es NULL para dispatch");
        return false;
    }
    
    log_info(get_logger(), "short_scheduler: Enviando dispatch PID=%d, PC=%d", pid, pc);
    
    // Usar DTO para enviar dispatch
    int sent_bytes = send_cpu_dispatch_request(cpu_connection->dispatch_socket_id, pid, pc);
    
    if (sent_bytes <= 0) {
        log_error(get_logger(), "short_scheduler: Error enviando dispatch");
        return false;
    }
    
    log_info(get_logger(), "short_scheduler: Dispatch enviado exitosamente");
    return true;
}

void run_short_scheduler(void) {
    log_info(get_logger(), "short_scheduler: Iniciando planificador de corto plazo");
    
    t_cpu_connection* cpu = get_free_cpu();
    
    if(cpu == NULL) {
        log_error(get_logger(), "short_scheduler: No hay CPUs disponibles");
        return;
    }

    lock_cpu(&cpu->cpu_exec_sem);

    bool cpu_is_processing = (cpu->current_process_executing != -1);
    
    if (cpu_is_processing) {
        // desalojo habilitado?
        bool preemption_enabled = should_preempt_current_process();
        
        if (!preemption_enabled) {
            // do nothing
            log_debug(get_logger(), "short_scheduler: CPU ocupada, preemption deshabilitado");
            unlock_cpu(&cpu->cpu_exec_sem);
            return;
        }
        // si preemption está habilitado, go to next
    }
    
    // next:
    // lock(READY_LIST)
    lock_ready_list();
    
    // find_next_ready() - is any in ready list?
    t_pcb* next_ready = get_next_process_to_dispatch_from_ready();
    
    if (next_ready == NULL) {
        // no hay procesos en ready
        log_debug(get_logger(), "short_scheduler: No hay procesos en READY");
        
        // lock(exec_list)
        lock_exec_list();
        
        // cpu is currently processing? (check for existence of current_processing in EXEC list)
        bool process_exists_in_exec = find_pcb_in_exec(cpu->current_process_executing);
        
        if (process_exists_in_exec) {
            // si existe en EXEC, no hacer nada
            log_debug(get_logger(), "short_scheduler: Proceso %d sigue en EXEC", cpu->current_process_executing);
            unlock_ready_list();
            unlock_exec_list();
            unlock_cpu(&cpu->cpu_exec_sem);
            return;
        } else {
            // no existe en EXEC (syscall lo movió a otro estado)
            log_info(get_logger(), "short_scheduler: Proceso %d ya no está en EXEC, liberando CPU", cpu->current_process_executing);
            cpu->current_process_executing = -1;
            unlock_ready_list();
            unlock_exec_list();
            unlock_cpu(&cpu->cpu_exec_sem);
            return;
        }
    }
    
    // si hay procesos en ready
    log_info(get_logger(), "short_scheduler: Proceso encontrado en READY: PID=%d", next_ready->pid);
    
    // desalojo enabled?
    bool preemption_enabled = should_preempt_current_process();
    
    if (preemption_enabled && cpu_is_processing) {
        // si se debe desalojar, entonces, usamos la cpu que encontramos arriba
        if (cpu != NULL) {
            log_info(get_logger(), "short_scheduler: Realizando preemption del proceso %d", cpu->current_process_executing);

            // send(interrupt) + receive(interrupt)
            t_cpu_interrupt* interrupt_info = send_and_receive_interrupt(cpu->interrupt_socket_id, cpu->current_process_executing);

            // lock(EXEC_LIST) - ya está lockeada implícitamente
            lock_exec_list();
            
            // move current_processing from EXEC to READY
            t_pcb* preempted_pcb = remove_pcb_from_exec(cpu->current_process_executing);
            if (preempted_pcb != NULL) {
                preempted_pcb->current_state = READY;
                preempted_pcb->pc = interrupt_info->pc; // Actualizar PC del PCB preemptado
                add_pcb_to_ready(preempted_pcb);
                log_info(get_logger(), "short_scheduler: Proceso %d movido de EXEC a READY por preemption", cpu->current_process_executing);
            }
            
            // Liberar la CPU anterior
            cpu->current_process_executing = -1;
        }
    } else {
        // lock(EXEC_LIST)
        lock_exec_list();
    }
    
    // move_process: move next READY to EXEC
    log_info(get_logger(), "short_scheduler: Moviendo proceso PID=%d de READY a EXEC", next_ready->pid);
    
    // Mover de READY a EXEC
    next_ready->current_state = EXEC;
    add_pcb_to_exec(next_ready);
    
    // send(dispatch, pid, pc)
    bool dispatch_success = send_dispatch_to_cpu(cpu, next_ready->pid, next_ready->pc);
    
    if (dispatch_success) {
        cpu->current_process_executing = next_ready->pid;
        log_info(get_logger(), "short_scheduler: Proceso PID=%d despachado exitosamente", next_ready->pid);
    } else {
        log_error(get_logger(), "short_scheduler: Error despachando proceso PID=%d", next_ready->pid);
        // Revertir cambios
        remove_pcb_from_exec(next_ready->pid);
        next_ready->current_state = READY;
        add_pcb_to_ready(next_ready);
    }
    
    // unlock(READY) + unlock(EXEC) + unlock(CPU_SEM)
    unlock_ready_list();
    unlock_exec_list();
    unlock_cpu_connections();
    
    log_info(get_logger(), "short_scheduler: Planificador de corto plazo completado");
}