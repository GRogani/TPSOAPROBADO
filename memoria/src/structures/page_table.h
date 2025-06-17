#ifndef MEMORIA_COLLECTIONS_H
#define MEMORIA_COLLECTIONS_H

#include <commons/collections/list.h>
#include "../utils.h"

typedef struct page_table_entry 
{
    bool is_last_level;
    union {
        uint32_t frame_number;
        t_list* next_level; 
    };

} t_page_table_entry;


typedef struct page_table {
    t_list* entries; 
    size_t num_entries;
} t_page_table;


#endif