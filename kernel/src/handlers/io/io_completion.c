#include "io_completion.h"

void* io_completion(void *thread_args)
{
    t_completion_thread_args *args = (t_completion_thread_args *)thread_args;

    LOG_INFO("Processing IO completion for device %s", args->device_name);

    // 1. Obtener la conexión IO para encontrar el PID del proceso que estaba ejecutando
    lock_io_connections();
    
    t_io_connection* connection = find_io_connection_by_socket(args->client_socket);
    if (connection == NULL)
    {
        LOG_ERROR("IO connection not found for device %s during completion", args->device_name);
        unlock_io_connections();
        goto cleanup;
    }

    uint32_t pid = connection->current_process_executing;
    if (pid == -1) {
        LOG_ERROR("No process was executing on device %s, but completion received", args->device_name);
        unlock_io_connections();
        goto cleanup;
    }

    // 2. Actualizar current_processing de la conexión a -1 (liberar dispositivo)
    connection->current_process_executing = -1;
    
    unlock_io_connections();

    // 3. Procesar solicitudes pendientes de IO ANTES de los schedulers
    t_pending_io_thread_args *pending_args = safe_malloc(sizeof(t_pending_io_thread_args));
    pending_args->pending_args.device_name = strdup(args->device_name);
    pending_args->pending_args.client_socket = args->client_socket;
    
    pthread_t pending_io_thread;
    int err = pthread_create(&pending_io_thread, NULL, process_pending_io_thread, pending_args);
    if (err != 0) {
        LOG_ERROR("Failed to create thread for processing pending IO for device %s", args->device_name);
        free(pending_args->pending_args.device_name);
        free(pending_args);
    } else {
        pthread_detach(pending_io_thread);
        LOG_INFO("Thread created for processing pending IO requests for device %s", args->device_name);
    }

    // 4. Buscar el proceso siguiendo el orden de locks: ready, blocked, susp_blocked, susp_ready
    t_pcb* pcb = NULL;
    bool found_in_blocked = false;
    bool found_in_susp_blocked = false;

    // Primero lockear en orden: ready, blocked, susp_blocked, susp_ready
    lock_ready_list();
    lock_blocked_list();
    lock_susp_blocked_list();
    lock_susp_ready_list();

    // Buscar en BLOCKED
    pcb = remove_pcb_from_blocked(pid);
    if (pcb != NULL) {
        found_in_blocked = true;
        LOG_INFO("Process PID %d found in BLOCKED, moving to READY", pid);
        add_pcb_to_ready(pcb);
    } else {
        // Si no se encontró en BLOCKED, buscar en SUSPENDED_BLOCKED
        pcb = remove_pcb_from_susp_blocked(pid);
        if (pcb != NULL) {
            found_in_susp_blocked = true;
            LOG_INFO("Process PID %d found in SUSPENDED_BLOCKED, moving to SUSPENDED_READY", pid);
            add_pcb_to_susp_ready(pcb);
        }
    }

    // Unlock en orden inverso
    unlock_susp_ready_list();
    unlock_susp_blocked_list();
    unlock_blocked_list();
    unlock_ready_list();

    // 5. Ejecutar schedulers según corresponda
    if (found_in_blocked) {
        // Proceso pasó de BLOCKED a READY - ejecutar planificador de corto plazo
        LOG_INFO("io_completion: running short scheduler after IO completion for PID %d", pid);
        run_short_scheduler();
    } else if (found_in_susp_blocked) {
        // Proceso pasó de SUSPENDED_BLOCKED a SUSPENDED_READY - ejecutar planificador de largo plazo
        LOG_INFO("io_completion: running long scheduler after IO completion for suspended PID %d", pid);
        bool process_moved_to_ready = run_long_scheduler();
        
        // Si el proceso pasó a READY, ejecutar corto plazo
        if (process_moved_to_ready) {
            LOG_INFO("io_completion: running short scheduler after long scheduler moved process to ready");
            run_short_scheduler();
        }
    } else {
        // No se encontró en ninguna lista, loggear error
        LOG_ERROR("Process PID %d not found in any blocked list during IO completion", pid);
    }

cleanup:
    free(args->device_name);
    free(args);
    pthread_exit(NULL);
}

void* process_pending_io_thread(void* thread_args)
{
    t_pending_io_thread_args *args = (t_pending_io_thread_args *)thread_args;
    
    // Ejecutar la rutina para procesar IO pendiente
    process_pending_io(args->pending_args);
    
    // La función process_pending_io ya se encarga de liberar device_name
    free(args);
    return NULL;
}