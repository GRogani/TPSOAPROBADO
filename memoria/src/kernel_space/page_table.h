#ifndef MEMORIA_COLLECTIONS_H
#define MEMORIA_COLLECTIONS_H

#include <commons/collections/list.h>
#include "../utils.h"

extern t_memoria_config memoria_config;

#define INVALID_FRAME_NUMBER -1

typedef struct page_table
{
    int32_t table_id;
    t_list *entries;
    size_t num_entries;
} t_page_table;

typedef struct page_table_entry
{
    bool is_last_level;
    int32_t next_table_id;
    union
    {
        int32_t frame_number;
        t_page_table *next_table;
    };
} t_page_table_entry;

static t_page_table_entry *init_page_table_entry(bool is_last_level);

static t_page_table *_create_table(int current_level);

void free_page_table(t_page_table *table);

t_page_table *init_page_table();

#endif