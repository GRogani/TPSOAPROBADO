#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../utils.h"
#include "page_table.h"
#include "user_space/frame_manager.h"

typedef struct process_metrics {
    uint32_t page_table_access_count;
    uint32_t instruction_requests_count;
    uint32_t swap_out_count;
    uint32_t swap_in_count;
    uint32_t memory_read_count;
    uint32_t memory_write_count;
} t_process_metrics;

typedef struct process_info {
    uint32_t pid;
    uint32_t process_size;
    bool is_suspended;
    t_list* instructions;
    t_page_table* page_table;
    t_process_metrics* metrics;
    t_list* allocated_frames;
} process_info;

// Helper to destroy a process_info structure, called by process_manager
void destroy_process_info(void* proc_void_ptr);

// Initializes the process manager (creates the internal process list)
void process_manager_init();

// Destroys the process manager (destroys the process list)
void process_manager_destroy();

// Loads script lines from a file path. Handled by process_manager now.
t_list* process_manager_load_script_lines(char* path);

// Creates a new process, assigns frames, builds page table, and adds to global list.
// Returns 0 on success, -1 on failure.
int process_manager_create_process(uint32_t pid, uint32_t size, char* script_path);

// Finds a process by PID. Returns pointer to process_info or NULL if not found.
// Callers should not modify the returned process_info directly.
process_info* process_manager_find_process(uint32_t pid);

// Removes a process by PID from the global list and frees its resources.
// Returns 0 on success, -1 if process not found.
int process_manager_delete_process(uint32_t pid);

#endif