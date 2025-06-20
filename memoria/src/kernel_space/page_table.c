#include "page_table.h" // Adjust path based on your project structure

t_page_table* create_page_table() {
    t_page_table* new_page_table = safe_malloc(sizeof(t_page_table));
    if (new_page_table == NULL) {
        return NULL;
    }
    new_page_table->entries = list_create();
    if (new_page_table->entries == NULL) {
        free(new_page_table);
        return NULL;
    }
    new_page_table->num_entries = 0;
    return new_page_table;
}

t_page_table_entry* create_page_table_entry(bool is_last_level) {
    t_page_table_entry* new_entry = safe_malloc(sizeof(t_page_table_entry));
    if (new_entry == NULL) {
        return NULL;
    }
    new_entry->is_last_level = is_last_level;
    if (is_last_level) {
        new_entry->frame_number = (uint32_t)-1; // Or some other default/invalid frame number
    } else {
        new_entry->next_level = list_create();
        if (new_entry->next_level == NULL) {
            free(new_entry);
            return NULL;
        }
    }
    return new_entry;
}

void add_page_table_entry(t_page_table* page_table, int index, t_page_table_entry* entry) {
    if (page_table == NULL || page_table->entries == NULL || entry == NULL) {
        return;
    }
    // This function assumes the list is grown to the 'index' or the index is valid for insertion.
    // For fixed-size tables, you might pre-allocate NULL entries and use list_replace.
    list_add_in_index(page_table->entries, index, entry);
    page_table->num_entries = list_size(page_table->entries);
}

t_page_table_entry* get_page_table_entry(t_page_table* page_table, int index) {
    if (page_table == NULL || page_table->entries == NULL) {
        return NULL;
    }
    if (index < 0 || index >= list_size(page_table->entries)) {
        return NULL;
    }
    return (t_page_table_entry*) list_get(page_table->entries, index);
}

void destroy_page_table_entry(void* entry_void_ptr) {
    t_page_table_entry* entry = (t_page_table_entry*) entry_void_ptr;
    if (entry == NULL) {
        return;
    }
    if (!entry->is_last_level) {
        list_destroy_and_destroy_elements(entry->next_level, destroy_page_table_entry);
    }
    free(entry);
}

void destroy_page_table(t_page_table* page_table) {
    if (page_table == NULL) {
        return;
    }
    list_destroy_and_destroy_elements(page_table->entries, destroy_page_table_entry);
    free(page_table);
}

// Recursive helper function to build the page table levels
static t_page_table* create_nested_page_table(int current_level, int total_levels, int entries_per_table) {
    if (current_level > total_levels) {
        return NULL;
    }

    t_page_table* new_table = create_page_table();
    if (new_table == NULL) {
        return NULL;
    }

    bool is_last_level_flag = (current_level == total_levels);

    for (int i = 0; i < entries_per_table; i++) {
        t_page_table_entry* entry = create_page_table_entry(is_last_level_flag);
        if (entry == NULL) {
            // Clean up already created entries and table
            list_destroy_and_destroy_elements(new_table->entries, destroy_page_table_entry);
            free(new_table);
            return NULL;
        }

        if (!is_last_level_flag) {
            t_page_table* nested_table = create_nested_page_table(current_level + 1, total_levels, entries_per_table);
            if (nested_table == NULL) {
                // Clean up already created entries and table
                destroy_page_table_entry(entry); // Free the current entry
                list_destroy_and_destroy_elements(new_table->entries, destroy_page_table_entry);
                free(new_table);
                return NULL;
            }
            entry->next_level = nested_table->entries; // The entry points to the list of entries of the nested table
            free(nested_table); // Free the outer t_page_table struct, as its 'entries' list is now held by the entry
        }
        list_add(new_table->entries, entry); // Use list_add as we're populating sequentially
    }
    new_table->num_entries = entries_per_table; // Set the total number of entries
    return new_table;
}

// Main initialization function
t_page_table* init_page_table(const t_memoria_config* config) {
    if (config == NULL || config->CANTIDAD_NIVELES <= 0 || config->ENTRADAS_POR_TABLA <= 0) {
        return NULL;
    }
    return create_nested_page_table(1, config->CANTIDAD_NIVELES, config->ENTRADAS_POR_TABLA);
}