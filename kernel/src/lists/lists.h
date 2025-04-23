#ifndef KERNEL_LISTS_H
#define KERNEL_LISTS_H


#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "../utils.h"

typedef struct t_cpu_connection {
    int interrupt_socket_id;
    int dispatch_socket_id;
} t_cpu_connection;

typedef struct t_io_connection {
    char* device_name;
    int socket_id;
} t_io_connection;

typedef struct t_io_queue{
    char* device_name;
    int pid;
} t_io_queue;

t_list* get_cpu_connections_list();
void initialize_global_lists();
void destroy_global_lists();
void io_connections_destroyer(void *ptr);
void io_queue_destroyer(void *ptr);
void cpu_connections_destroyer(void *ptr);

#endif