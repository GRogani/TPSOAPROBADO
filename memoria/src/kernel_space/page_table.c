#include "page_table.h"

_Atomic int32_t next_table_id = 1;

static t_page_table_entry *init_page_table_entry(bool is_last_level)
{
    t_page_table_entry *entry = safe_malloc(sizeof(t_page_table_entry));

    entry->is_last_level = is_last_level;
    entry->next_table_id = 0;

    if (is_last_level)
    {
        entry->frame_number = INVALID_FRAME_NUMBER;
    }
    else
    {
        entry->next_table = NULL;
    }

    return entry;
}

static t_page_table *_create_table(int current_level)
{
    t_page_table *table = safe_malloc(sizeof(t_page_table));

    table->table_id = next_table_id++;

    table->num_entries = memoria_config.ENTRADAS_POR_TABLA;

    table->entries = list_create();

    bool is_last_level = (current_level == memoria_config.CANTIDAD_NIVELES - 1);

    for (size_t i = 0; i < table->num_entries; i++)
    {
        t_page_table_entry *entry = init_page_table_entry(is_last_level);

        if (!is_last_level)
        {
            entry->next_table = _create_table(current_level + 1);
            entry->next_table_id = entry->next_table->table_id;
        }

        list_add(table->entries, entry);
    }

    return table;
}

t_page_table *init_page_table()
{
    t_page_table* table = _create_table(0);
    next_table_id = 1; // Reset for future processes
    return table;
}

void free_page_table(t_page_table *table)
{
    if (!table)
    {
        return;
    }

    for (size_t i = 0; i < table->num_entries; i++)
    {
        t_page_table_entry *entry = list_get(table->entries, i);

        if (!entry->is_last_level && entry->next_table != NULL)
        {
            free_page_table(entry->next_table);
        }

        free(entry);
    }

    list_destroy(table->entries);
    free(table);
}