#include "io_completion.h"

void* io_completion(void *thread_args)
{
    t_completion_thread_args *args = (t_completion_thread_args *)thread_args;

    LOG_INFO("io_completion: Processing IO completion for device %s for PID %d", args->device_name, args->pid);

    // 1. Usar el PID proporcionado por el módulo IO
    int32_t pid = args->pid;

    // Obtener la conexión IO para liberar el dispositivo
    lock_io_connections();
    
    t_io_connection* connection = find_io_connection_by_socket(args->client_socket);
    if (connection == NULL)
    {
        LOG_ERROR("IO connection not found for device %s during completion", args->device_name);
        unlock_io_connections();
        goto cleanup;
    }

    // Primero lockear en orden: ready, blocked, susp_blocked, susp_ready
    lock_ready_list();
    lock_blocked_list();
    lock_susp_blocked_list();
    lock_susp_ready_list();

    // 4. Buscar el proceso siguiendo el orden de locks: ready, blocked, susp_blocked, susp_ready
    t_pcb *pcb = NULL;
    bool found_in_blocked = false;
    bool found_in_susp_blocked = false;

    // Buscar en BLOCKED
    pcb = remove_pcb_from_blocked(pid);
    if (pcb != NULL) {
        found_in_blocked = true;
        LOG_OBLIGATORIO("## (%d) finalizó IO y pasa a READY", pid);
        add_pcb_to_ready(pcb);
    } else {
        // Si no se encontró en BLOCKED, buscar en SUSPENDED_BLOCKED
        pcb = remove_pcb_from_susp_blocked(pid);
        if (pcb != NULL) {
            found_in_susp_blocked = true;
            LOG_INFO("io_completion: Process PID %d found in SUSPENDED_BLOCKED, moving to SUSPENDED_READY", pid);
            add_pcb_to_susp_ready(pcb);
        } else {
            LOG_ERROR("## (%d) NO SE ENCONTRÓ EL PROCESO NI EN BLOCKED NI EN SUSP BLOCKED MIENTRAS SE RECIBIÓ UNA IO COMPLETION.", pid);
        }
    }

    // 2. Actualizar current_processing de la conexión a -1 (liberar dispositivo)
    connection->current_process_executing = -1;

    // Unlock en orden inverso
    unlock_susp_ready_list();
    unlock_susp_blocked_list();
    unlock_blocked_list();
    unlock_ready_list();
    unlock_io_connections();

    // Procesar solicitudes pendientes de IO ANTES de los schedulers
    t_pending_io_args pending_args;
    pending_args.device_name = strdup(args->device_name);
    pending_args.client_socket = args->client_socket;
    process_pending_io(pending_args);
    pthread_t process_pending_io_t;
    if (pthread_create(&process_pending_io_t, NULL, process_pending_io_thread, NULL) != 0)
    {
        LOG_ERROR("IO_COMPLETION: Failure pthread_create");
    }
    pthread_detach(process_pending_io_t);

    // 5. Ejecutar schedulers según corresponda
    if (found_in_blocked) {
        // Proceso pasó de BLOCKED a READY - ejecutar planificador de corto plazo
        pthread_t thread;
        if (pthread_create(&thread, NULL, process_scheduler_for_blocked, NULL) != 0)
        {
            LOG_ERROR("IO_COMPLETION: Failure pthread_create");
        }
        pthread_detach(thread);
    } else if (found_in_susp_blocked) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, process_schedulers_for_susp_blocked, NULL) != 0)
        {
            LOG_ERROR("IO_COMPLETION: Failure pthread_create");
        }
        pthread_detach(thread);
    } else {
        // No se encontró en ninguna lista, loggear error
        LOG_ERROR("Process PID %d not found in any blocked list during IO completion", pid);
    }

cleanup:
    free(args->device_name);
    free(args);
    pthread_exit(NULL);
}

void process_scheduler_for_blocked()
{
    LOG_INFO("io_completion: running short scheduler after IO completion");
    run_short_scheduler();
}

void process_schedulers_for_susp_blocked() {
    LOG_INFO("io_completion: running long scheduler after IO completion for suspended");
    bool process_moved_to_ready = run_long_scheduler();

    // Si el proceso pasó a READY, ejecutar corto plazo
    if (process_moved_to_ready)
    {
        LOG_INFO("io_completion: running short scheduler after long scheduler moved process to ready");
        run_short_scheduler();
    }
}

void *process_pending_io_thread(void *thread_args)
{
    t_pending_io_thread_args *args = (t_pending_io_thread_args *)thread_args;
    
    // Ejecutar la rutina para procesar IO pendiente
    
    
    // La función process_pending_io ya se encarga de liberar device_name
    free(args);
    return NULL;
}