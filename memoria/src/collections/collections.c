#include "collections.h"

#include "page_table_manager.h"
#include <stdlib.h>
#include <commons/string.h>

t_dictionary* page_tables_by_pid = NULL;

void init_collections(size_t entries_per_level) {
    page_tables_by_pid = dictionary_create();
}

t_page_table* create_page_table_recursive(size_t levels_remaining, size_t entries_per_level) {
    t_page_table* table = malloc(sizeof(t_page_table));
    table->entries = list_create();
    table->num_entries = entries_per_level;

    for (size_t i = 0; i < entries_per_level; i++) {
        t_page_table_entry* entry = malloc(sizeof(t_page_table_entry));
        entry->is_last_level = (levels_remaining == 1);

        if (entry->is_last_level) {
            entry->frame_number = 0;
        } else {
            entry->next_level = create_page_table_recursive(levels_remaining - 1, entries_per_level)->entries;
        }

        list_add(table->entries, entry);
    }

    return table;
}

void destroy_page_table_recursive(t_page_table* table) {
    void destroy_entry(void* element) {
        t_page_table_entry* entry = (t_page_table_entry*)element;

        if (!entry->is_last_level && entry->next_level != NULL) {
            
            t_page_table temp = { .entries = entry->next_level, .num_entries = list_size(entry->next_level) };
            destroy_page_table_recursive(&temp);
        }

        free(entry);
    }

    list_destroy_and_destroy_elements(table->entries, destroy_entry);
    free(table);
}

void associate_page_table_with_pid(uint32_t pid, t_page_table* table) {
    char* pid_str = string_itoa(pid);
    dictionary_put(page_tables_by_pid, pid_str, table);
    free(pid_str);
}

t_page_table* get_page_table_by_pid(uint32_t pid) {
    char* pid_str = string_itoa(pid);
    t_page_table* table = dictionary_get(page_tables_by_pid, pid_str);
    free(pid_str);
    return table;
}

void destroy_all_page_tables() {
    void destroy_table(void* table) {
        destroy_page_table_recursive((t_page_table*)table);
    }

    dictionary_destroy_and_destroy_elements(page_tables_by_pid, destroy_table);
}
