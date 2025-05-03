#ifndef KERNEL_LISTS_H
#define KERNEL_LISTS_H


#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "../utils.h"
#include "repository/new_list.h"

typedef struct t_cpu_connection {
    int interrupt_socket_id;
    int dispatch_socket_id;
    int id;
} t_cpu_connection;

typedef struct t_io_connection {
    bool connected;
    char* device_name;
    int socket_id;
} t_io_connection;

typedef struct t_io_queue {
    int pid;
    bool processing;
} t_io_queue;

typedef struct t_io_requests {
    char* device_name;
    t_io_queue* requests_queue;
} t_io_requests;

t_list* get_cpu_connections_list();
t_list* get_new_list();

void initialize_global_lists();
void destroy_global_lists();
void io_connections_destroyer(void *ptr);
void io_requests_destroyer(void *ptr);
void io_requests_queue_destroyer(void *ptr);
void cpu_connections_destroyer(void *ptr);

#endif