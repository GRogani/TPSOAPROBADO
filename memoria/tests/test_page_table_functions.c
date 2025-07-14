#include "test_page_table_functions.h"
#include <math.h>

static uint32_t next_table_id = 0;

// Función para inicializar una entrada de tabla de páginas
t_page_table_entry *init_page_table_entry(bool is_last_level)
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

// Función para crear una tabla de páginas
t_page_table *create_table(int current_level)
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
            entry->next_table = create_table(current_level + 1);
            entry->next_table_id = entry->next_table->table_id;
        }

        list_add(table->entries, entry);
    }

    return table;
}

// Función para inicializar una tabla de páginas
t_page_table *init_page_table()
{
    return create_table(0);
}

// Función para liberar una tabla de páginas
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

// Implementación de collect_last_level_tables
void collect_last_level_tables(t_page_table *root_table, stack_item_t *result, int *result_size, int max_result_size)
{
  // Crear una queue dinámica para BFS usando commons
  t_queue *bfs_queue = queue_create();

  // Crear el primer elemento para la cola
  queue_item_t *root_item = malloc(sizeof(queue_item_t));
  root_item->table = root_table;
  root_item->level = 0;

  // Agregar la tabla raíz a la queue
  queue_push(bfs_queue, root_item);

  // Mientras la queue no esté vacía
  while (!queue_is_empty(bfs_queue))
  {
    // Sacar un elemento de la queue
    queue_item_t *current_item = queue_pop(bfs_queue);
    t_page_table *current_table = current_item->table;
    int current_level = current_item->level;

    // Si es una tabla de último nivel, la agregamos al resultado
    if (current_level == memoria_config.CANTIDAD_NIVELES - 1)
    {
      if (*result_size < max_result_size)
      {
        result[*result_size].table = current_table;
        result[*result_size].level = current_level;
        result[*result_size].entry_index = 0;
        (*result_size)++;
      }
      free(current_item);
      continue;
    }

    // Si no es una tabla de último nivel, agregamos sus hijos a la queue
    for (int i = 0; i < current_table->num_entries; i++)
    {
      t_page_table_entry *entry = list_get(current_table->entries, i);
      if (entry && entry->next_table)
      {
        queue_item_t *new_item = malloc(sizeof(queue_item_t));
        new_item->table = entry->next_table;
        new_item->level = current_level + 1;
        queue_push(bfs_queue, new_item);
      }
    }

    // Liberamos el elemento actual
    free(current_item);
  }

  // Liberamos la cola
  queue_destroy(bfs_queue);
}

// Implementación de assign_frames_to_process
bool assign_frames_to_process(t_page_table *root_table, void **frames_for_process, int pages_needed)
{
  // Validación de parámetros
  if (!root_table || !frames_for_process || pages_needed <= 0)
  {
    return false;
  }

  // Calculamos el número máximo posible de tablas de último nivel
  int max_last_level_tables = pow(memoria_config.ENTRADAS_POR_TABLA, memoria_config.CANTIDAD_NIVELES - 1);
  stack_item_t *last_level_tables = malloc(sizeof(stack_item_t) * max_last_level_tables);
  int num_last_level_tables = 0;

  // Recopilamos todas las tablas de último nivel usando BFS
  collect_last_level_tables(root_table, last_level_tables, &num_last_level_tables, max_last_level_tables);

  // Inicializamos contador de frames asignados
  int frame_index = 0;

  // Procesamos todas las tablas de último nivel y asignamos frames a sus entradas
  for (int i = 0; i < num_last_level_tables && frame_index < pages_needed; i++)
  {
    t_page_table *last_level_table = last_level_tables[i].table;

    for (int j = 0; j < last_level_table->num_entries && frame_index < pages_needed; j++)
    {
      t_page_table_entry *entry = list_get(last_level_table->entries, j);
      if (entry)
      {
        // Asignamos el frame
        uint32_t *frame_number = frames_for_process[frame_index]; // Acceso directo al arreglo
        if (frame_number)
        {
          entry->frame_number = *frame_number;
          frame_index++;
        }
      }
    }
  }

  // Liberamos la memoria
  free(last_level_tables);

  // Verificamos si asignamos todos los frames requeridos
  return frame_index == pages_needed;
}
