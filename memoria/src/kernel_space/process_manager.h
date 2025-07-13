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
    t_list* swap_pages_info;  // Estructura para rastrear páginas en swap
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

// Updates the page table of a process with new frame assignments
// This is used during swap in to reassign frames to the page table
bool update_process_page_table(process_info* proc, t_list* new_frames);

// Checks if a process with the given PID exists
bool process_manager_process_exists(uint32_t pid);

// Get the global list of processes (for debugging and special cases)
t_list* process_manager_get_process_list();

bool assign_frames_to_process(t_page_table *root_table, t_list *frames_for_process, int pages_needed);

bool assign_frames_recursive(t_page_table *current_table, t_list *frames, int *current_frame_idx_ptr, int pages_needed);

#endif