#ifndef MMU_H
#define MMU_H

#include "../utils.h"

typedef struct t_user_memory
{
    void* user_space;
    size_t size;
} t_user_memory;

typedef struct t_page_table_entry {
    bool is_last_level;

    union { // union usa un solo espacio, entonces o es uno o es otro
        uint32_t frame_number;           
        struct t_page_table* next_level; 
    };
    
} t_page_table_entry;

typedef struct t_page_table {
    t_page_table_entry* entries;
    size_t num_entries;
} t_page_table;

void init_user_memory(int size);
void destroy_user_memory();

#endif

