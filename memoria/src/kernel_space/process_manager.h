#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../utils.h"
#include "page_table.h"
#include "user_space/frame_manager.h"
#include "./page_table.h"

static t_list *global_process_list = NULL;

extern t_memoria_config memoria_config;

typedef struct process_metrics {
    int32_t page_table_access_count;
    int32_t instruction_requests_count;
    int32_t swap_out_count;
    int32_t swap_in_count;
    int32_t memory_read_count;
    int32_t memory_write_count;
} t_process_metrics;

typedef struct process_info {
    int32_t pid;
    int32_t process_size;
    bool is_suspended;
    t_list* instructions;
    t_page_table* page_table;
    t_process_metrics* metrics;
    t_list* allocated_frames;
} process_info;

void destroy_process_info(void* proc_void_ptr);

void process_manager_init();

void process_manager_destroy();

process_info* process_manager_find_process(int32_t pid);

bool process_manager_delete_process(int32_t pid);

t_list* process_manager_get_process_list();

#endif