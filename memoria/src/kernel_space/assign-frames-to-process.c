#include "assign-frames-to-process.h"


static void collect_last_level_tables(t_page_table *root_table, stack_item_t *result, int *result_size, int max_result_size)
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

// Versión iterativa usando BFS para recopilar todas las tablas de último nivel
bool assign_frames(t_page_table *root_table, t_list* frames_for_process, int pages_needed)
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
        int32_t *frame_number = list_get(frames_for_process, frame_index); // Acceso directo al arreglo
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