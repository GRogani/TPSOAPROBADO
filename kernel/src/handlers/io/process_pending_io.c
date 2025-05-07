#include "process_pending_io.h"

void process_pending_io(t_pending_io_args args)
{
    t_pending_io_args thread_args = (t_pending_io_args) args;

    log_info(get_logger(), "Processing pending IO for device %s", thread_args->device_name);

    lock_io_requests_link();

    t_io_requests_link* request_link = find_io_request_by_device_name(thread_args->device_name);
    if(request_link == NULL) {
        log_error(get_logger(), "Could not found request_link element!!");
        goto free_request_link;
    }

    lock_io_requests_list(&request_link->io_requests_list_semaphore);

    void* request_element = find_first_element_for_socket(request_link->requests_list, thread_args->client_socket);

    if(request_element == NULL) {
        log_info(get_logger(), "There is no requests assigned to the connection, trying to assign...");
        t_io_request* element_not_assigned = find_first_element_without_assign(request_link->requests_list);

        if(element_not_assigned == NULL) {
            log_info(get_logger(), "There is no pending requests for device %s", thread_args->device_name);
            goto free_all;
        }

        int err = send_io_request(thread_args->client_socket, element_not_assigned->pid, element_not_assigned->sleep_time);
        if(err == -1) {
            log_error(get_logger(), "Could not sent socket %d request io message for process %d", thread_args->client_socket, element_not_assigned->pid);
            goto free_all;
        }

        assign_connection_to_request(request_link->requests_list, thread_args->client_socket);
        log_info(get_logger(), "Assigned connection %d to request %d", thread_args->client_socket, element_not_assigned->pid);
    }

    // si hay algun elemento asignado al socket, no debemos hacer nada porque el device ya está ejecutando.

    goto free_all;


    free_all:
        unlock_io_requests_list(&request_link->io_requests_list_semaphore);
        goto free_request_link;
        return;
    

    free_request_link: 
        unlock_io_requests_link();
        free(thread_args->device_name);
        return;
    }