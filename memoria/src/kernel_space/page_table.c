#include "page_table.h" // Adjust path based on your project structure

_Atomic uint32_t next_available_table_id = 1; // Comenzamos desde 1 para evitar confusiones con ID 0

t_page_table *create_page_table()
{
    t_page_table *new_page_table = safe_malloc(sizeof(t_page_table));
    if (new_page_table == NULL)
    {
        return NULL;
    }
    new_page_table->entries = list_create();
    if (new_page_table->entries == NULL)
    {
        free(new_page_table);
        return NULL;
    }
    
    // Asignar un ID único y registrarlo en el log
    uint32_t assigned_id = next_available_table_id++;
    new_page_table->table_id = assigned_id;
    new_page_table->num_entries = 0;
    
    LOG_DEBUG("PAGE_TABLE: Nueva tabla creada con ID: %u", assigned_id);
    return new_page_table;
}

t_page_table_entry *create_page_table_entry(bool is_last_level)
{
    t_page_table_entry *new_entry = safe_malloc(sizeof(t_page_table_entry));
    new_entry->is_last_level = is_last_level;
    new_entry->next_table_id = 0; // Inicializar a 0 por defecto
    
    if (is_last_level)
    {
        new_entry->frame_number = INVALID_FRAME_NUMBER; // Placeholder, despues lo inicializamos
    }
    else
    {
        new_entry->next_table = NULL; // Will be set later when creating nested table
    }
    return new_entry;
}

void add_page_table_entry(t_page_table *page_table, int index, t_page_table_entry *entry)
{
    if (page_table == NULL || page_table->entries == NULL || entry == NULL)
    {
        return;
    }
    list_add_in_index(page_table->entries, index, entry);
    page_table->num_entries = list_size(page_table->entries);
}

t_page_table_entry *get_page_table_entry(t_page_table *page_table, int index)
{
    if (page_table == NULL)
    {
        LOG_ERROR("get_page_table_entry: page_table is NULL");
        return NULL;
    }
    if (page_table->entries == NULL)
    {
        LOG_ERROR("get_page_table_entry: page_table->entries is NULL");
        return NULL;
    }

    int list_size_val = list_size(page_table->entries);
    LOG_INFO("get_page_table_entry: index=%d, list_size=%d", index, list_size_val);

    if (index < 0 || index >= list_size_val)
    {
        LOG_ERROR("get_page_table_entry: index %d out of bounds [0, %d)", index, list_size_val);
        return NULL;
    }

    // Verificar que la lista no esté corrupta
    if (page_table->entries->head == NULL && list_size_val > 0)
    {
        LOG_ERROR("get_page_table_entry: list has size %d but head is NULL", list_size_val);
        return NULL;
    }

    t_page_table_entry *entry = (t_page_table_entry *)list_get(page_table->entries, index);
    if (entry == NULL)
    {
        LOG_ERROR("get_page_table_entry: list_get returned NULL for index %d", index);
    }
    return entry;
}

void destroy_page_table_entry(void *entry_void_ptr)
{
    t_page_table_entry *entry = (t_page_table_entry *)entry_void_ptr;
    if (entry == NULL)
    {
        return;
    }
    if (!entry->is_last_level && entry->next_table != NULL)
    {
        destroy_page_table(entry->next_table);
    }
    free(entry);
}

void destroy_page_table(t_page_table *page_table)
{
    if (page_table == NULL)
    {
        return;
    }
    list_destroy_and_destroy_elements(page_table->entries, destroy_page_table_entry);
    free(page_table);
}

// Recursive helper function to build the page table levels
t_page_table *create_nested_page_table(int current_level, int total_levels, int entries_per_table)
{
    if (current_level > total_levels)
    {
        return NULL;
    }

    LOG_INFO("create_nested_page_table: level=%d, total_levels=%d, entries_per_table=%d",
             current_level, total_levels, entries_per_table);

    t_page_table *new_table = create_page_table();
    if (new_table == NULL)
    {
        LOG_ERROR("create_nested_page_table: failed to create page table");
        return NULL;
    }

    bool is_last_level_flag = (current_level == total_levels);
    LOG_INFO("create_nested_page_table: is_last_level=%s", is_last_level_flag ? "true" : "false");

    for (int i = 0; i < entries_per_table; i++)
    {
        t_page_table_entry *entry = create_page_table_entry(is_last_level_flag);
        if (entry == NULL)
        {
            LOG_ERROR("create_nested_page_table: failed to create entry %d", i);
            // Clean up already created entries and table
            list_destroy_and_destroy_elements(new_table->entries, destroy_page_table_entry);
            free(new_table);
            return NULL;
        }

        if (!is_last_level_flag)
        {
            t_page_table *nested_table = create_nested_page_table(current_level + 1, total_levels, entries_per_table);
            if (nested_table == NULL)
            {
                LOG_ERROR("create_nested_page_table: failed to create nested table for entry %d", i);
                // Clean up already created entries and table
                destroy_page_table_entry(entry); // Free the current entry
                list_destroy_and_destroy_elements(new_table->entries, destroy_page_table_entry);
                free(new_table);
                return NULL;
            }
            entry->next_table_id = nested_table->table_id; // Guardar el ID de la tabla
            entry->next_table = nested_table; // Store the complete nested table structure
            LOG_INFO("create_nested_page_table: entry %d assigned next_table_id=%u with %zu entries",
                     i, nested_table->table_id, nested_table->num_entries);
        }
        list_add(new_table->entries, entry); // Use list_add as we're populating sequentially
        LOG_INFO("create_nested_page_table: added entry %d to table", i);
    }
    new_table->num_entries = entries_per_table; // Set the total number of entries
    LOG_INFO("create_nested_page_table: created table with %zu entries", new_table->num_entries);
    return new_table;
}

// Main initialization function
t_page_table *init_page_table(const t_memoria_config *config)
{
    if (config == NULL || config->CANTIDAD_NIVELES <= 0 || config->ENTRADAS_POR_TABLA <= 0)
    {
        return NULL;
    }
    return create_nested_page_table(1, config->CANTIDAD_NIVELES, config->ENTRADAS_POR_TABLA);
}

/**
 * @brief Resetea todos los números de frame en una tabla de páginas multinivel
 * @param current_table Tabla de páginas actual
 * @param total_levels Total de niveles en la jerarquía
 * @param current_level Nivel actual siendo procesado
 * @return true si éxito, false si error
 */
bool reset_page_table_frames(t_page_table *current_table, int total_levels, int current_level)
{
    if (current_table == NULL || current_table->entries == NULL)
    {
        LOG_ERROR("Error: Tabla actual o entradas es NULL en reset_page_table_frames.");
        return false;
    }

    bool is_last_level_of_hierarchy = (current_level == total_levels);

    for (int i = 0; i < current_table->num_entries; i++)
    {
        t_page_table_entry *entry = get_page_table_entry(current_table, i);
        if (entry == NULL)
        {
            LOG_ERROR("Error: Entrada de tabla de paginas NULL en indice %d de nivel %d.", i, current_level);
            return false;
        }

        if (is_last_level_of_hierarchy)
        {
            entry->frame_number = INVALID_FRAME_NUMBER; // Reset to invalid frame number
        }
        else
        {
            if (entry->next_table != NULL)
            {
                if (!reset_page_table_frames(entry->next_table, total_levels, current_level + 1))
                {
                    return false;
                }
            }
        }
    }
    return true;
}