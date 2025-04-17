#ifndef GLOBALS_KERNEL_H
#define GLOBALS_KERNEL_H

#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <utils/logger/logger.h>
#include <utils/config/config.h>

typedef struct t_io_connection {
    char* device_name;
    int socket_id;
} t_io_connection;

typedef struct t_io_queue{
    char* device_name;
    int pid;
} t_io_queue;

typedef struct t_cpu_connection {
    int interrupt_socket_id;
    int dispatch_socket_id;
} t_cpu_connection;

extern t_config* config; // defined by main
extern t_kernel_config kernel_config; // defined by main
extern t_list* io_connections_list; // defined by main
extern t_queue* io_requests_queue; // defined by main
extern t_list* cpu_connections_list; // defined by main

#endif