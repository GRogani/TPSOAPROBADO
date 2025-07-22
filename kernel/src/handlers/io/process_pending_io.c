#include "process_pending_io.h"

bool process_pending_io(t_pending_io_args args)
{
    LOG_INFO("process_pending_io: Processing pending IO for device %s", args.device_name);

    lock_io_connections();

    t_io_connection_status connection_found = get_io_connection_status_by_device_name(args.device_name);
    if (!connection_found.found)
    {
        LOG_INFO("process_pending_io: There are no existing connections for device %s", args.device_name);
        unlock_io_connections();
        free(args.device_name);
        return false;
    }

    lock_io_requests_link();

    t_io_requests_link *request_link = find_io_request_by_device_name(args.device_name);
    if (request_link == NULL)
    {
        LOG_ERROR("Could not found request_link element.");
        goto free_request_link_connection;
    }

    lock_io_requests_queue(&request_link->io_requests_queue_semaphore);

    // Verificar si el dispositivo está realmente ocupado comprobando si el PID actual 
    // está en las listas BLOCKED o SUSP_BLOCKED
    int32_t current_pid = connection_found.connection->current_process_executing;
    bool is_truly_busy = false;
    
    if (current_pid != -1) {
        // Necesitamos bloquear las listas para verificar si el proceso está realmente bloqueado
        lock_blocked_list();
        lock_susp_blocked_list();
        
        void* pcb_in_blocked = find_pcb_in_blocked(current_pid);
        t_pcb* pcb_in_susp_blocked = find_pcb_in_susp_blocked(current_pid);
        
        is_truly_busy = (pcb_in_blocked != NULL || pcb_in_susp_blocked != NULL);
        
        unlock_susp_blocked_list();
        unlock_blocked_list();
    }
    
    if (is_truly_busy) {
        LOG_INFO("process_pending_io: IO device is truly busy with PID %d for device %s", current_pid, args.device_name);
        goto free_all; // como está busy, dejamos que se mande a ejecutar cuando se libere.
    } else if (current_pid != -1) {
        LOG_INFO("process_pending_io: IO device marked as busy but PID %d is not in blocked lists. Marking as available.", current_pid);
        // No es necesario hacer nada aquí, ya que actualizaremos current_process_executing más adelante
    }

    // hay una conexion libre, asignamos la conexion al proceso

    // obtenemos el primer elemento de la lista de solicitudes
    void *request = get_next_request_in_queue(request_link->io_requests_queue);
    if (request == NULL)
    {
        LOG_INFO("process_pending_io: There is no pending requests for device %s", args.device_name);
        goto free_all;
    }
    t_io_request *io_request = (t_io_request *)request;

    // mandamos la solicitud al modulo IO
    int err = send_io_operation_package(args.client_socket, io_request->pid, io_request->sleep_time);
    if (err == -1)
    {
        LOG_ERROR("Could not sent socket %d request io message for process %d", args.client_socket, io_request->pid);
        goto free_all;
    }

    // si se pudo enviar la solicitud, la eliminamos de la lista de solicitudes y liberamos el elemento
    void *element = pop_next_request_in_queue(request_link->io_requests_queue);
    if (element == NULL)
    {
        LOG_ERROR("FATAL ERROR: Could not pop io_request from queue, there is no elements");
        goto free_all;
    }

    // actualizamos el current_processing de la conexion (luego sirve por si se desconecta, para saber que proceso estaba ejecutando y pasarlo a EXIT)
    connection_found.connection->current_process_executing = io_request->pid;

    free(element);

    goto free_all;

free_all:
    unlock_io_requests_queue(&request_link->io_requests_queue_semaphore);
    goto free_request_link_connection;

free_request_link_connection:
    unlock_io_requests_link();
    unlock_io_connections();
    free(args.device_name);
    return true;
}