#ifndef MEMORIA_PROCESS_H
#define MEMORIA_PROCESS_H

#include "page_table.h"

typedef struct process_metrics 
{
    uint32_t page_table_access_count;
    uint32_t instruction_requests_count;
    uint32_t swap_out_count; // Bajadas a SWAP
    uint32_t swap_in_count;  // Subidas a Memoria Principal
    uint32_t memory_read_count;
    uint32_t memory_write_count;
} t_process_metrics;


typedef struct process_info {
    uint32_t pid;
    uint32_t process_size; // Total size requested by the process
    char* instructions; // Pseudocode lines (list of char*)
    t_page_table* page_table; // Root of the process's page table
    t_process_metrics* metrics; // Metrics for this process
    
} process_info;

process_info* find_process_by_pid(uint32_t pid);
t_list* load_script_lines(char* path);
int create_process(uint32_t pid, uint32_t size, char* script_path);
void destroy_proc_memory(void* proc_void_ptr); 

void init_process(int socket, t_package* package);
void get_instruction(int socket, t_package* package);
void delete_process(int socket, t_package *package);
void get_free_space(int socket);

#endif