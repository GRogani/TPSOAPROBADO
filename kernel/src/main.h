#ifndef GLOBAL_SHARED_MAIN
#define GLOBAL_SHARED_MAIN

#define PUERTO_IO_ESCUCHA "30002"
#define PUERTO_CPU_DISPATCH_ESCUCHA "30003"
#define PUERTO_CPU_INTERRUPT_ESCUCHA "30004"

#include <utils/logger/logger.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <config/config.h>
#include <utils/socket/server.h>
#include <pthread.h>


typedef struct {
    char* device_name;
    int socket_id;
} t_io_connection;

typedef struct {
    char* device_name;
    int pid;
} t_io_queue;

typedef struct {
    int interrupt_socket_id;
    int dispatch_socket_id;
} t_cpu_connection;

t_log* logger;

t_list* io_connections_list;
t_queue* io_requests_queue;
t_list* cpu_connections_list;

t_kernel_config* kernel_config;


#endif