#include "medium_scheduler.h"

void run_medium_scheduler(uint32_t  pid, uint32_t timer){
    /**
   * Aca va la implementacion del algoritmo de MEDIANO plazo
   * 1. recibe un pid y un tiempo, hace sleep de ese tiempo
   * 2. terminado el sleep, blockeamos la lista de BLOCKED y verificamos si está el proceso en la lista de BLOCKED
   * 3. si existe, pasamos el proceso a SUSP. BLOCKED
   * 4. hacemos el swap con la memoria
   * 5. corremos el largo plazo (porque acabamos de liberar espacio en memoria del sistema)
   * 6. si no existe, loggeamos y no hacemos nada (probablemente ya se resolvió el proceso)
   * 7. Mando a correr el corto plazo, ya que hay un nuevo proceso en READY potencialmente para ejecutar
   */

    LOG_INFO("MEDIUM_SCHEDULER: Tiempo de espera de [%d] para PID [%u]", timer, pid);
    sleep(timer);
    lock_blocked_list();
    if(!find_pcb_in_blocked(pid)) {
        unlock_blocked_list();
        LOG_ERROR("MEDIUM_SCHEDULER: PID [%d] no está en BLOCKED.", pid);
        return;
    }
    // BLOCKED -> BLOCKED_SUSPEND
    t_pcb* pcb = remove_pcb_from_blocked(pid);
    unlock_blocked_list();
    LOG_INFO("MEDIUM_SCHEDULER: Cambio de Estado BLOCKED->SUSPENDED_BLOCKED del PID = %d.", pid);

    int memory_socket = connect_to_memory(&kernel_config);
    if (memory_socket == -1){
        LOG_ERROR("MEDIUM_SCHEDULER: Falló la conexión con Memoria.");
        lock_blocked_list();
        add_pcb_to_blocked(pcb);
        unlock_blocked_list();
        return;
    }
    LOG_INFO("MEDIUM_SCHEDULER: Solicitando Servicio de Swapping para suspender PID = %d.", pid);
    send_swap_package(memory_socket, pid);

    // Espero la respuesta
    t_package* package = recv_package(memory_socket);
    if(package == NULL){
        LOG_ERROR("MEDIUM_SCHEDULER: No se pudo conectar con Memoria para suspender PID = %d", pid);
        lock_blocked_list();
        add_pcb_to_blocked(pcb);
        unlock_blocked_list();
        return;
    }
    if(read_confirmation_package(package) != 0) { // Success = 0
        LOG_WARNING("MEDIUM_SCHEDULER: Falló el SWAP del PID [%d]", pid);
        LOG_DEBUG("Resultado del paquete enviado = %d", read_confirmation_package(package));
        // TODO: si falla el SWAP, por ahora hago esto SUSPEND_BLOCKED -> BLOCKED
        lock_blocked_list();
        add_pcb_to_blocked(pcb);
        unlock_blocked_list();

        disconnect_from_memory(memory_socket);
        destroy_package(package);
        return;
    }
    LOG_INFO("MEDIUM_SCHEDULER: Swap Exitoso del PID [%d]", pid);
    disconnect_from_memory(memory_socket);
    destroy_package(package);

    lock_susp_blocked_list();
    add_pcb_to_susp_blocked(pcb);
    unlock_susp_blocked_list();

    // Llamo al Planificador de Largo Plazo
    LOG_INFO("MEDIUM_SCHEDULER: Llamando a planificador de largo plazo para admitir nuevos procesos.");
    if (!run_long_scheduler()) {
        LOG_WARNING("MEDIUM_SCHEDULER: El planificador de largo plazo NO puede admitir nuevos procesos.");
        return;
    }        
    LOG_INFO("MEDIUM_SCHEDULER: El planificador de largo plazo admitió a un nuevo PID [%d].", pid);
    LOG_INFO("MEDIUM_SCHEDULER: Llamando al planificador de corto plazo.");
    run_short_scheduler();
}