#include "collections.h"

static t_dictionary *io_connections_dict;
static t_dictionary *io_requests_link_dict;

static t_dictionary *cpu_connections_dict;
static t_list *new_list;
static t_list *ready_list;
static t_list *exec_list;
static t_list *blocked_list;
static t_list *susp_blocked_list;
static t_list *susp_ready_list;
static t_list *exit_list;

t_dictionary *get_cpu_connections_dict()
{
    return cpu_connections_dict;
}

t_list *get_new_list()
{
    return new_list;
}

t_dictionary *get_io_connections_dict()
{
    return io_connections_dict;
}

t_dictionary *get_io_requests_link_dict()
{
    return io_requests_link_dict;
}

void initialize_global_lists()
{

    initialize_repository_io_connections();
    io_connections_dict = dictionary_create();

    initialize_repository_cpu_connections();
    cpu_connections_dict = dictionary_create();

    initialize_repository_new();
    new_list = list_create();

    ready_list = list_create();
    exec_list = list_create();
    blocked_list = list_create();
    susp_blocked_list = list_create();
    susp_ready_list = list_create();
    exit_list = list_create();

    initialize_repository_io_requests_link();
    io_requests_link_dict = dictionary_create();

    if (
        io_connections_dict == NULL ||
        io_requests_link_dict == NULL ||
        cpu_connections_dict == NULL ||
        new_list == NULL ||
        ready_list == NULL ||
        exec_list == NULL ||
        blocked_list == NULL ||
        susp_blocked_list == NULL ||
        susp_ready_list == NULL ||
        exit_list == NULL)
    {
        LOG_ERROR("Some of the lists/queues failed to create");
        exit(EXIT_FAILURE);
    }
}

void destroy_global_lists()
{
    dictionary_destroy_and_destroy_elements(io_connections_dict, io_connections_destroyer);
    dictionary_destroy_and_destroy_elements(cpu_connections_dict, cpu_connections_destroyer);
    dictionary_destroy_and_destroy_elements(io_requests_link_dict, io_requests_destroyer);
}

void destroy_global_repositories()
{
    destroy_repository_new();
    destroy_repository_io_connections();
    destroy_repository_io_requests_link();
}

void io_connections_destroyer(void *ptr)
{
    t_io_connection *connection = (t_io_connection *)ptr;
    free(connection);
}

void io_requests_queue_destroyer(void *ptr)
{
    t_io_request *request_el = (t_io_request *)ptr;
    free(request_el);
}

void io_requests_destroyer(void *ptr)
{
    t_io_requests_link *io_request = (t_io_requests_link *)ptr;

    destroy_repository_io_requests_queue(
        &io_request->io_requests_queue_semaphore);

    queue_destroy_and_destroy_elements(io_request->io_requests_queue, io_requests_queue_destroyer);

    free(io_request);
}

void cpu_connections_destroyer(void *ptr)
{
    t_cpu_connection *connection = (t_cpu_connection *)ptr;
    free(connection);
}