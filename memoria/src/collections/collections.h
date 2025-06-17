#ifndef MEMORIA_COLLECTIONS_H
#define MEMORIA_COLLECTIONS_H

#include <commons/collections/list.h>
#include "../utils.h"

typedef struct page_table_entry 
{
    bool is_last_level;
    union {
        uint32_t frame_number;
        t_list* next_level; // page_table_entry*
    };
} t_page_table_entry;

// Page table: contains a list of entries and how many it has
typedef struct page_table {
    t_list* entries; // list of t_page_table_entry*
    size_t num_entries;
} t_page_table;


// Global dictionary: PID -> t_page_table*
extern t_dictionary* page_tables_by_pid;

// Initializes the global dictionary
void init_collections(size_t entries_per_level);

// Creates a new hierarchical page table with `levels` depth
t_page_table* create_page_table_recursive(size_t levels, size_t entries_per_level);

// Frees memory recursively
void destroy_page_table_recursive(t_page_table* table);

// Associates a page table with a PID
void associate_page_table_with_pid(uint32_t pid, t_page_table* table);

// Retrieves a page table associated with a PID
t_page_table* get_page_table_by_pid(uint32_t pid);

// Frees the global dictionary and all tables inside
void destroy_all_page_tables();


#endif