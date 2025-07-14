#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "test_utils.h"
#include "test_page_table_functions.h"

// Mock de la configuración global
t_memoria_config memoria_config = {
    .TAM_PAGINA = 256,
    .ENTRADAS_POR_TABLA = 4,  // Valor pequeño para facilitar las pruebas
    .CANTIDAD_NIVELES = 3,    // 3 niveles jerárquicos para la prueba
    .RETARDO_MEMORIA = 0,
    .RETARDO_SWAP = 0
};

// Función auxiliar para crear frames de mock
t_list *create_mock_frames(int num_frames, uint32_t starting_frame) {
  t_list *frames = list_create();
  
  for (int i = 0; i < num_frames; i++) {
    uint32_t *frame = malloc(sizeof(uint32_t));
    *frame = starting_frame + i;
    list_add(frames, frame);
  }
  
  return frames;
}

// Función auxiliar para liberar frames de mock
void free_mock_frames(void **frames, int num_frames) {
    for (int i = 0; i < num_frames; i++) {
        free(frames[i]);
    }
    free(frames);
}

void test_collect_last_level_tables(t_page_table *root_table, stack_item_t *result, int *result_size, int max_result_size)
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
bool test_assign_frames_to_process(t_page_table *root_table, t_list *frames_for_process, int pages_needed)
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
  test_collect_last_level_tables(root_table, last_level_tables, &num_last_level_tables, max_last_level_tables);

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
        uint32_t *frame_number = list_get(frames_for_process, frame_index); // Acceso directo al arreglo
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

// Función auxiliar para imprimir una entrada de tabla de páginas
void print_page_table_entry(t_page_table_entry *entry, int level, int entry_index, char *path) {
    if (!entry) return;
    
    if (entry->is_last_level) {
        if (entry->frame_number == INVALID_FRAME_NUMBER) {
            printf("      Entrada %d: Frame = INVALID (0)\n", entry_index);
        } else {
            printf("      Entrada %d: Frame = ASIGNADO (%d)\n", entry_index, entry->frame_number);
        }
    } else {
        char new_path[256];
        if (path[0] == '\0') {
            sprintf(new_path, "%d", entry_index);
        } else {
            sprintf(new_path, "%s->%d", path, entry_index);
        }
        
        printf("    Entrada %d: Siguiente tabla ID = %d\n", entry_index, entry->next_table_id);
        printf("    Tabla ID: %d (Path: %s) Nivel: %d\n", entry->next_table->table_id, new_path, level + 1);
        
        for (int i = 0; i < entry->next_table->num_entries; i++) {
            t_page_table_entry *next_entry = list_get(entry->next_table->entries, i);
            print_page_table_entry(next_entry, level + 1, i, new_path);
        }
    }
}

// Función auxiliar para imprimir una tabla de páginas completa
void print_page_table(t_page_table *table) {
    if (!table) return;
    
    printf("Tabla raíz ID: %d\n", table->table_id);
    
    for (int i = 0; i < table->num_entries; i++) {
        t_page_table_entry *entry = list_get(table->entries, i);
        char path[2] = {0};
        sprintf(path, "%d", i);
        
        printf("  Entrada %d: Siguiente tabla ID = %d\n", i, entry->next_table_id);
        printf("  Tabla ID: %d (Path: %s) Nivel: %d\n", entry->next_table->table_id, path, 1);
        
        for (int j = 0; j < entry->next_table->num_entries; j++) {
            t_page_table_entry *next_entry = list_get(entry->next_table->entries, j);
            char new_path[256];
            sprintf(new_path, "%d->%d", i, j);
            print_page_table_entry(next_entry, 1, j, path);
        }
    }
}

// Función auxiliar para contar los frames asignados en una tabla de páginas
int count_assigned_frames(t_page_table *table) {
    if (!table) return 0;
    
    int count = 0;
    t_queue *queue = queue_create();
    
    // Crear estructura auxiliar para el BFS
    typedef struct {
        t_page_table *table;
        int level;
    } bfs_item_t;
    
    // Insertar la raíz en la cola
    bfs_item_t *root_item = malloc(sizeof(bfs_item_t));
    root_item->table = table;
    root_item->level = 0;
    queue_push(queue, root_item);
    
    // BFS para contar frames
    while (!queue_is_empty(queue)) {
        bfs_item_t *current = queue_pop(queue);
        t_page_table *current_table = current->table;
        int current_level = current->level;
        
        if (current_level == memoria_config.CANTIDAD_NIVELES - 1) {
            // Es una tabla de último nivel, contar frames asignados
            for (int i = 0; i < current_table->num_entries; i++) {
                t_page_table_entry *entry = list_get(current_table->entries, i);
                if (entry && entry->frame_number != INVALID_FRAME_NUMBER) {
                    count++;
                }
            }
        } else {
            // No es una tabla de último nivel, agregar hijos a la cola
            for (int i = 0; i < current_table->num_entries; i++) {
                t_page_table_entry *entry = list_get(current_table->entries, i);
                if (entry && entry->next_table) {
                    bfs_item_t *next_item = malloc(sizeof(bfs_item_t));
                    next_item->table = entry->next_table;
                    next_item->level = current_level + 1;
                    queue_push(queue, next_item);
                }
            }
        }
        
        free(current);
    }
    
    // Liberar la cola
    while (!queue_is_empty(queue)) {
        free(queue_pop(queue));
    }
    queue_destroy(queue);
    
    return count;
}

// Implementación de safe_malloc requerida por las funciones
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        printf("Error: No se pudo asignar memoria\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int main() {
    printf("===== Test de Asignación de Frames a Proceso (Función Original) =====\n\n");
    
    // Crear un proceso con una estructura de tabla de páginas jerárquica
    printf("=== Test Básico con la Función Original ===\n");
    printf("Inicializando tabla de páginas con %d niveles y %d entradas por tabla\n", 
           memoria_config.CANTIDAD_NIVELES, memoria_config.ENTRADAS_POR_TABLA);
    
    // Inicializar la tabla de páginas para el proceso 0
    t_page_table *root_table = init_page_table();
    
    // Calcular el total de páginas posibles
    int total_pages = pow(memoria_config.ENTRADAS_POR_TABLA, memoria_config.CANTIDAD_NIVELES);
    printf("Total de páginas posibles: %d\n", total_pages);
    
    // Crear los frames mock para el proceso
    int pages_needed = 10; // Necesitaremos 10 frames
    printf("Creando %d frames de mock para asignar al proceso 0...\n", pages_needed);
    t_list* mock_frames = create_mock_frames(pages_needed, 1000);
    
    // Llamar a la función original para asignar frames
    printf("Llamando a la función original assign_frames_to_process...\n");
    bool result = test_assign_frames_to_process(root_table, mock_frames, pages_needed);
    
    // Verificar el resultado
    int frames_assigned = count_assigned_frames(root_table);
    printf("Resultado de la asignación: %s\n", result ? "ÉXITO" : "ERROR");
    printf("Frames asignados: %d (esperados: %d)\n\n", frames_assigned, pages_needed);
    
    // Imprimir la estructura de la tabla de páginas después de la asignación
    printf("Estructura de la tabla de páginas después de la asignación:\n\n");
    print_page_table(root_table);
    
    // Liberar recursos
    free_mock_frames(mock_frames, pages_needed);
    free_page_table(root_table);
    
    // Caso límite: intentar asignar más frames de los que pueden caber
    printf("\n=== Test de Caso Límite con la Función Original ===\n");
    printf("Inicializando tabla de páginas con %d niveles y %d entradas por tabla\n", 
           2, 2); // Tabla más pequeña para el test de límite
    
    // Cambiar la configuración temporalmente para este test
    int old_niveles = memoria_config.CANTIDAD_NIVELES;
    int old_entradas = memoria_config.ENTRADAS_POR_TABLA;
    memoria_config.CANTIDAD_NIVELES = 2;
    memoria_config.ENTRADAS_POR_TABLA = 2;
    
    // Inicializar una nueva tabla de páginas más pequeña
    root_table = init_page_table();
    
    // Calcular el total de páginas posibles en esta configuración
    total_pages = pow(memoria_config.ENTRADAS_POR_TABLA, memoria_config.CANTIDAD_NIVELES);
    printf("Total de páginas posibles: %d\n", total_pages);
    
    // Intentar asignar más frames de los que pueden caber
    pages_needed = total_pages + 2;
    printf("Intentando asignar %d frames (más de los %d posibles)...\n", pages_needed, total_pages);
    mock_frames = create_mock_frames(pages_needed, 2000);
    
    // Llamar a la función original nuevamente
    result = assign_frames_to_process(root_table, mock_frames, pages_needed);
    
    // Verificar el resultado
    frames_assigned = count_assigned_frames(root_table);
    printf("Resultado de la asignación: %s\n", result ? "ÉXITO" : "ERROR");
    printf("Frames asignados: %d (máximo posible: %d)\n\n", frames_assigned, total_pages);
    
    // Imprimir la estructura de la tabla de páginas después de la asignación
    printf("Estructura de la tabla de páginas después de la asignación:\n\n");
    print_page_table(root_table);
    
    // Restaurar la configuración original
    memoria_config.CANTIDAD_NIVELES = old_niveles;
    memoria_config.ENTRADAS_POR_TABLA = old_entradas;
    
    // Liberar recursos
    free_mock_frames(mock_frames, pages_needed);
    free_page_table(root_table);
    
    printf("\nTodos los tests completados.\n");
    
    return 0;
}
