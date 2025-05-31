#include "process_pending_io.h"

void process_pending_io(t_pending_io_args args)
{

    LOG_INFO("Processing pending IO for device %s", args.device_name);

    lock_io_connections();
    lock_io_requests_link();

    t_io_requests_link *request_link = find_io_request_by_device_name(args.device_name);
    if (request_link == NULL)
    {
        LOG_ERROR("Could not found request_link element.");
        goto free_request_link_connection;
    }

    lock_io_requests_queue(&request_link->io_requests_queue_semaphore);

    void *connection = find_free_connection_from_device_name(args.device_name);
    t_io_connection *connection_found = (t_io_connection *)connection;
    if (connection_found == NULL)
    {
        LOG_INFO("There is no free connections for device %s", args.device_name);
        goto free_all;
    }

    // hay una conexion libre, asignamos la conexion al proceso

    // obtenemos el primer elemento de la lista de solicitudes
    void *request = get_next_request_in_queue(request_link->io_requests_queue);
    if (request == NULL)
    {
        LOG_INFO("There is no pending requests for device %s", args.device_name);
        goto free_all;
    }
    t_io_request *io_request = (t_io_request *)request;

    // mandamos la solicitud al modulo IO
    int err = send_io_request(args.client_socket, io_request->pid, io_request->sleep_time);
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
    connection_found->current_process_executing = io_request->pid;

    free(element);

    goto free_all;

free_all:
    unlock_io_requests_queue(&request_link->io_requests_queue_semaphore);
    goto free_request_link_connection;

free_request_link_connection:
    unlock_io_requests_link();
    unlock_io_connections();
    free(args.device_name);
    return;
}