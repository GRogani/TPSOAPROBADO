#include "medium_scheduler.h"

// TODO: en realidad el timer lo saco del archivo, no?
void run_medium_scheduler(uint32_t  pid, uint32_t timer){
    /**
   * Aca va la implementacion del algoritmo de MEDIANO plazo
   * 1. recibe un pid y un tiempo, hace sleep de ese tiempo
   * 2. terminado el sleep, blockeamos la lista de BLOCKED y verificamos si está el proceso en la lista de BLOCKED
   * 3. si existe, pasamos el proceso a SUSP. BLOCKED
   * 4. hacemos el swap con la memoria
   * 5. corremos el largo plazo (porque acabamos de liberar espacio en memoria del sistema)
   * 6. si no existe, loggeamos y no hacemos nada (probablemente ya se resolvió el proceso)
   */

    // TODO: usar el nuevo looger
    log_info(get_logger(), "MEDIUM_SCHEDULER: Tiempo de espera de [%d] para PID [%u]", timer, pid);
    sleep(timer);
    lock_blocked_list();
    if(!find_pcb_in_blocked(pid)) {
        unlock_blocked_list();
        log_info(get_logger(), "MEDIUM_SCHEDULER: PID [%d] no está en BLOCKED", pid);
        return NULL;
    }
    unlock_blocked_list();
    // BLOCKED -> BLOCKED_SUSPEND
    t_pcb* pcb = remove_pcb_from_blocked(pid);
    pcb->current_state = SUSP_BLOCKED;
    unlock_blocked_list();
    log_info(get_logger(), "MEDIUM_SCHEDULER: Cambio de Estado BLOCKED->SUSPENDED_BLOCKED del PID = %d.", pid);

    int memory_socket = connect_to_memory(&kernel_config);
    if (memory_socket == -1){
        log_error(get_logger(), "MEDIUM_SCHEDULER: No se pudo conectar con Memoria para suspender PID = %d.", pid);
        return NULL;
    }
    log_info(get_logger(), "MEDIUM_SCHEDULER: Solicitando Servicio de Swapping para suspender PID = %d.", pid);
    // TODO: la memoria conoce a todos los procesos o lo tengo que mandar el PCB?
    send_memory_suspend_process(memory_socket, pid);

    // TODO: espero la respuseta (qué me tendría que llegar)?
    disconnect_from_memory(memory_socket);

    lock_susp_blocked_list();
    add_pcb_to_susp_blocked(pcb);
    unlock_susp_blocked_list();

    // Llamo al Planificador de Largo Plazo
    log_info(get_logger(), "MEDIUM_SCHEDULER: Llamando a planificador de largo plazo para admitir nuevos procesos.");
    if (!run_long_scheduler()) {
        LOG_ERROR("MEDIUM_SCHEDULER: El planificador de largo plazo no puede admitir nuevos procesos.");
    }
    log_info(get_logger(), "MEDIUM_SCHEDULER: Finalización del PID [%d].", pid);
}