#ifndef TEST_PAGE_TABLE_FUNCTIONS_H
#define TEST_PAGE_TABLE_FUNCTIONS_H

#include "test_utils.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <stdlib.h>

typedef struct page_table
{
    uint32_t table_id;
    t_list *entries;
    size_t num_entries;
} t_page_table;

typedef struct page_table_entry
{
    bool is_last_level;
    uint32_t next_table_id;
    union
    {
        int32_t frame_number;
        t_page_table *next_table;
    };
} t_page_table_entry;

// Estructura para cola de BFS
typedef struct {
    t_page_table *table;
    int level;
} queue_item_t;

// Estructura para almacenar tablas de último nivel
typedef struct
{
  t_page_table *table;
  int level;
  int entry_index;
} stack_item_t;

// Prototipos de funciones
t_page_table_entry *init_page_table_entry(bool is_last_level);
t_page_table *create_table(int current_level);
t_page_table *init_page_table();
void free_page_table(t_page_table *table);
bool assign_frames_to_process(t_page_table *root_table, void **frames_for_process, int pages_needed);
void collect_last_level_tables(t_page_table *root_table, stack_item_t *result, int *result_size, int max_result_size);

#endif // TEST_PAGE_TABLE_FUNCTIONS_H
