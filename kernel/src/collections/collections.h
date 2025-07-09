#ifndef KERNEL_COLLECTIONS_H
#define KERNEL_COLLECTIONS_H

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <enums/Eprocess_state.h>
#include "../utils.h"
#include "repository/process/new_list.h"
#include "repository/process/ready_list.h"
#include "repository/process/exec_list.h"
#include "repository/process/blocked_list.h"
#include "repository/process/exit_list.h"
#include "repository/process/susp_blocked_list.h"
#include "repository/process/susp_ready_list.h"
#include "repository/io/io_connections.h"
#include "repository/io/io_requests_link.h"
#include "repository/io/io_requests_queue.h"
#include "repository/cpu/cpu_connections.h"
#include "repository/pcb/pcb.h"

typedef struct t_cpu_connection
{
    unsigned int id;
    int interrupt_socket_id;
    int dispatch_socket_id;
    int32_t current_process_executing;
    sem_t cpu_exec_sem;
} t_cpu_connection;

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

t_dictionary *get_cpu_connections_dict();
t_list *get_new_list();
t_list *get_ready_list();
t_list *get_exec_list();
t_list *get_blocked_list();
t_list *get_exit_list();
t_list *get_susp_blocked_list();
t_list *get_susp_ready_list();
t_dictionary *get_io_connections_dict();
t_dictionary *get_io_requests_link_dict();

void initialize_global_lists();
void destroy_global_lists();
void destroy_global_repositories();
void io_connections_destroyer(void *ptr);
void io_requests_destroyer(void *ptr);
void io_requests_queue_destroyer(void *ptr);
void cpu_connections_destroyer(void *ptr);
void pcb_destroyer(void *ptr);

#endif