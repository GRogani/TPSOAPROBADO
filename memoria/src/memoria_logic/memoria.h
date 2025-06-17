#ifndef MEMORIA_H
#define MEMORIA_H

#include "../utils.h"
#include "structures/page_table.h"

typedef struct process_metrics {
    uint32_t page_table_access_count;
    uint32_t instruction_requests_count;
    uint32_t swap_out_count; // Bajadas a SWAP
    uint32_t swap_in_count;  // Subidas a Memoria Principal
    uint32_t memory_read_count;
    uint32_t memory_write_count;
} t_process_metrics;


typedef struct proc_memory {
    uint32_t pid;
    uint32_t process_size; // Total size requested by the process
    t_list* instructions; // Pseudocode lines (list of char*)
    t_page_table* page_table; // Root of the process's page table
    t_process_metrics* metrics; // Metrics for this process
    // TODO: Add fields for tracking allocated frames/pages in user_memory_space
} proc_memory;

typedef struct global_memory_state {
    t_list* processes; // List of proc_memory* for all active processes
    pthread_mutex_t processes_mutex; // Mutex for thread-safe access to 'processes' list
    t_memoria_config* config; // Global configuration pointer
    t_log* main_logger; // Global logger instance
    // TODO: Add other global resources here, like the SWAP manager, free frame list, etc.
} global_memory_state;

extern global_memory_state global_memory; // Declare the global instance


void init_global_memory_state(t_memoria_config* config);
void destroy_global_memory_state();

proc_memory* find_process_by_pid(uint32_t pid);
t_list* load_script_lines(char* path);
int create_process(uint32_t pid, uint32_t size, char* script_path);
void destroy_proc_memory(void* proc_void_ptr); 


void init_process(int socket, t_package* package);
void get_instruction(int socket, t_package* package);
void delete_process(int socket, t_package *package);
void get_free_space(int socket); 
#endif 