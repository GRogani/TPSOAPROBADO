#ifndef KERNEL_COLLECTIONS_H
#define KERNEL_COLLECTIONS_H

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include "../utils.h"
#include "repository/new_list.h"
#include "repository/io_connections.h"
#include "repository/io_requests_link.h"
#include "repository/io_requests_queue.h"

typedef struct t_cpu_connection
{
    int interrupt_socket_id;
    int dispatch_socket_id;
    int id;
} t_cpu_connection;

typedef struct t_io_connection
{
    char *device_name;
    int current_process_executing;
} t_io_connection;

typedef struct t_io_request
{
    int pid;
    int sleep_time;
} t_io_request;

typedef struct t_io_requests_link_list
{
    sem_t io_requests_queue_semaphore;
    t_queue *io_requests_queue;
} t_io_requests_link;

t_list *get_cpu_connections_list();
t_list *get_new_list();
t_dictionary *get_io_connections_dict();
t_dictionary *get_io_requests_link_dict();

void initialize_global_lists();
void destroy_global_lists();
void destroy_global_repositories();
void io_connections_destroyer(void *ptr);
void io_requests_destroyer(void *ptr);
void io_requests_queue_destroyer(void *ptr);
void cpu_connections_destroyer(void *ptr);

#endif