#include "process_pending_io.h"

void process_pending_io(void* args) {
    t_pending_io_thread_args* thread_args = (t_pending_io_thread_args*) args;

    lock_io_requests_link();
    lock_io_requests_list();

    t_io_requests_link* request_link = find_io_requests_by_device_name(thread_args->device_name);
    if(request_link == NULL) {
        LOG_ERROR("Could not found request_link element!!");
        goto free_args;
    }

    void* request_element = find_first_element_for_socket(request_link->requests_list, thread_args->client_socket);

    if(request_element == NULL) {
        t_io_request* element_not_assigned = find_first_element_without_assign(request_link->requests_list);

        if(element_not_assigned == NULL) {
            // no hay elementos para asignar
            goto free_args;
        }

        int err = send_io_request(thread_args->client_socket, io_request->pid, io_request->sleep_time);
        if(err == -1) {
            LOG_ERROR("Could not sent socket %d request io message for process %d", thread_args->client_socket, io_request->pid);
            goto free_args;
        }

        void* assigned_element = assign_connection_to_request(request_link->requests_list, thread_args->client_socket);

    } else {
        // si hay algun elemento asignado al socket, no debemos hacer nada porque el device ya está ejecutando.
    }

    goto free_args;


    free_args:
        unlock_io_requests_list();
        unlock_io_requests_link();
        free(thread_args->device_name);
        free(thread_args);
        return;
}