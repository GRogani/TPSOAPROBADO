#include "medium_scheduler.h"

void run_medium_scheduler(int32_t pid)
{
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

    LOG_INFO("Tiempo de espera de %d milisegundos para PID %u", kernel_config.sleep_time, pid);
    usleep(kernel_config.sleep_time * 1000); // Conversión milisegundos -> macrosegundos de la función usleep

    lock_blocked_list();

    t_pcb *blocked_pid_found = find_pcb_in_blocked(pid);
    if (!blocked_pid_found)
    {
        unlock_blocked_list();
        LOG_WARNING("El PID %d no esta en BLOCKED", pid);
        return;
    }

    uint64_t time_in_blocked = total_time_ms(blocked_pid_found->state_start_time_ms);
    LOG_INFO("Tiempo en estado BLOCKED: %lu ms", time_in_blocked);
    if (time_in_blocked < kernel_config.sleep_time)
    {
        LOG_INFO("El PID %d no necesita ser suspendido, tiempo en BLOCKED %lu ms es menor al tiempo de espera %d ms", pid, time_in_blocked, kernel_config.sleep_time);
        unlock_blocked_list();
        return;
    }

    lock_susp_blocked_list();

    // BLOCKED -> SUSPEND_BLOCKED
    t_pcb *pcb = remove_pcb_from_blocked(pid);
    LOG_INFO("Cambio de estado de BLOCKED a SUSPENDED_BLOCKED para PID %d", pid);
    add_pcb_to_susp_blocked(pcb);
    unlock_blocked_list();
    unlock_susp_blocked_list();

    if (pcb->size != 0)
    {
        LOG_OBLIGATORIO("## (%d) enviando swap a la memoria para suspenderlo", pid);
        int memory_socket = connect_to_memory(&kernel_config);
        if (memory_socket == -1)
        {
            LOG_ERROR("Fallo la conexion con Memoria");
            lock_blocked_list();
            lock_susp_blocked_list();
            pcb = remove_pcb_from_susp_blocked(pid);
            add_pcb_to_blocked(pcb);
            unlock_susp_blocked_list();
            unlock_blocked_list();
            return;
        }
        LOG_INFO("Solicitando servicio de swapping para suspender PID %d", pid);
        send_swap_package(memory_socket, pid);

        t_package *package = recv_package(memory_socket);
        if (package == NULL)
        {
            LOG_ERROR("No se pudo conectar con Memoria para suspender PID %d", pid);
            lock_blocked_list();
            lock_susp_blocked_list();
            pcb = remove_pcb_from_susp_blocked(pid);
            add_pcb_to_blocked(pcb);
            unlock_susp_blocked_list();
            unlock_blocked_list();
            return;
        }
        if (!read_confirmation_package(package))
        { 
            LOG_WARNING("Fallo el SWAP del PID %d", pid);
            // TODO: si falla el SWAP, por ahora hago esto SUSPEND_BLOCKED -> BLOCKED
            lock_blocked_list();
            lock_susp_blocked_list();
            pcb = remove_pcb_from_susp_blocked(pid);
            add_pcb_to_blocked(pcb);
            unlock_susp_blocked_list();
            unlock_blocked_list();

            disconnect_from_memory(memory_socket);
            destroy_package(package);
            return;
        }

        LOG_INFO("Swap exitoso para PID %d", pid);
        LOG_OBLIGATORIO("## Swap finalizó con exito para PID %d", pid);
        disconnect_from_memory(memory_socket);
        destroy_package(package);
    }

    // Llamo al Planificador de Largo Plazo
    LOG_INFO("Llamando a planificador de largo plazo para admitir nuevos procesos");
    if (!run_long_scheduler())
    {
        LOG_INFO("El planificador de largo plazo no puede admitir nuevos procesos");
        return;
    }
    LOG_INFO("El planificador de largo plazo admitio a un nuevo PID %d", pid);
    LOG_INFO("Llamando al planificador de corto plazo");
    run_short_scheduler();
}
