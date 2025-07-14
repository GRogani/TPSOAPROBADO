#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <commons/collections/queue.h> // Incluimos la biblioteca de commons para la cola

// Definiciones básicas para el test aislado
#define INVALID_FRAME_NUMBER 0xFFFFFFFF

// Estructura de configuración simulada
typedef struct {
    uint32_t TAM_PAGINA;
    uint32_t ENTRADAS_POR_TABLA;
    uint32_t CANTIDAD_NIVELES;
} t_memoria_config;

// Configuración global simulada para las pruebas
t_memoria_config memoria_config = {
    .TAM_PAGINA = 256,
    .ENTRADAS_POR_TABLA = 4,  // Valor pequeño para facilitar las pruebas
    .CANTIDAD_NIVELES = 3     // 3 niveles jerárquicos para la prueba
};

// Estructura para una entrada de tabla de páginas
typedef struct page_table_entry {
    bool is_last_level;
    uint32_t next_table_id;
    union {
        struct page_table* next_table;  // Para niveles no-finales
        uint32_t frame_number;          // Para el último nivel
    };
} t_page_table_entry;

// Estructura para una tabla de páginas
typedef struct page_table {
    uint32_t table_id;
    uint32_t num_entries;
    void** entries;  // Simular una lista simple
} t_page_table;

// Estructura para la pila iterativa
typedef struct {
    t_page_table *table;
    int level;
    int entry_index;  // Para saber qué entrada de la tabla estamos procesando
} stack_item_t;

// Variable estática para generar IDs únicos para las tablas de páginas
static uint32_t next_table_id = 0;

// Reemplazo simple de safe_malloc para pruebas
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Funciones simplificadas para manejar listas (renombradas para evitar conflictos con commons)
void** simple_list_create() {
    return (void**)safe_malloc(sizeof(void*) * memoria_config.ENTRADAS_POR_TABLA);
}

void simple_list_add(void** list, void* element) {
    // Buscamos la primera posición vacía y añadimos el elemento
    for (uint32_t i = 0; i < memoria_config.ENTRADAS_POR_TABLA; i++) {
        if (list[i] == NULL) {
            list[i] = element;
            return;
        }
    }
}

void* simple_list_get(void** list, size_t index) {
    if (index >= memoria_config.ENTRADAS_POR_TABLA) {
        return NULL;
    }
    return list[index];
}

size_t simple_list_size(void** list) {
    size_t count = 0;
    for (uint32_t i = 0; i < memoria_config.ENTRADAS_POR_TABLA; i++) {
        if (list[i] != NULL) {
            count++;
        }
    }
    return count;
}

void simple_list_destroy(void** list) {
    free(list);
}

// Función auxiliar para crear una entrada de tabla de páginas
static t_page_table_entry* init_page_table_entry(bool is_last_level) {
    t_page_table_entry* entry = safe_malloc(sizeof(t_page_table_entry));
    
    entry->is_last_level = is_last_level;
    entry->next_table_id = 0;
    
    if (is_last_level) {
        entry->frame_number = INVALID_FRAME_NUMBER;  // Frame inválido por defecto
    } else {
        entry->next_table = NULL;  // La tabla siguiente no está inicializada aún
    }
    
    return entry;
}

// Función auxiliar para crear recursivamente las tablas de páginas
static t_page_table* _create_table(int current_level) {
    t_page_table* table = safe_malloc(sizeof(t_page_table));
    
    // Asignar un ID único a la tabla
    table->table_id = next_table_id++;
    
    // Configurar el número de entradas según la configuración
    table->num_entries = memoria_config.ENTRADAS_POR_TABLA;
    
    // Crear la lista de entradas
    table->entries = simple_list_create();
    
    // Inicializar todas las entradas a NULL
    for (uint32_t i = 0; i < table->num_entries; i++) {
        table->entries[i] = NULL;
    }
    
    // Determinar si este es el último nivel
    bool is_last_level = (current_level == memoria_config.CANTIDAD_NIVELES - 1);
    
    // Inicializar todas las entradas de la tabla
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = init_page_table_entry(is_last_level);
        
        // Si no es el último nivel, crear recursivamente la tabla del siguiente nivel
        if (!is_last_level) {
            entry->next_table = _create_table(current_level + 1);
            entry->next_table_id = entry->next_table->table_id;
        }
        
        simple_list_add(table->entries, entry);
    }
    
    return table;
}

t_page_table* init_page_table() {
    // Inicializar la tabla de primer nivel
    return _create_table(0);
}

// Función para obtener una entrada específica de la tabla de páginas
t_page_table_entry* get_page_table_entry(t_page_table* table, size_t index) {
    if (!table || index >= table->num_entries) {
        return NULL;
    }
    
    return simple_list_get(table->entries, index);
}

// Función para liberar la memoria de una tabla de páginas
void free_page_table(t_page_table* table) {
    if (!table) {
        return;
    }
    
    // Liberar todas las entradas
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = simple_list_get(table->entries, i);
        
        if (entry) {
            // Si no es el último nivel y hay una tabla siguiente, liberarla recursivamente
            if (!entry->is_last_level && entry->next_table != NULL) {
                free_page_table(entry->next_table);
            }
            
            // Liberar la entrada
            free(entry);
        }
    }
    
    // Liberar la lista de entradas y la tabla
    simple_list_destroy(table->entries);
    free(table);
}

// Función para imprimir el estado de una entrada de tabla de páginas
void print_page_table_entry(t_page_table_entry* entry, int level, int index) {
    if (!entry) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    if (entry->is_last_level) {
        printf("Entrada %d: Frame = %s (%u)\n", 
               index, 
               (entry->frame_number == INVALID_FRAME_NUMBER) ? "INVALID" : "ASIGNADO",
               entry->frame_number == INVALID_FRAME_NUMBER ? 0 : entry->frame_number);
    } else {
        printf("Entrada %d: Siguiente tabla ID = %u\n", index, entry->next_table_id);
    }
}

// Función recursiva para imprimir el estado de las tablas de páginas
void print_page_table_recursive(t_page_table* table, int level, int path[], int path_len) {
    if (!table) {
        return;
    }

    // Imprimir indentación según el nivel
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    // Imprimir el path hasta esta tabla (solo si no es la raíz)
    if (level > 0) {
        printf("Tabla ID: %u (Path: ", table->table_id);
        for (int i = 0; i < path_len; i++) {
            printf("%d", path[i]);
            if (i < path_len - 1) printf("->");
        }
        printf(") Nivel: %d\n", level);
    } else {
        printf("Tabla raíz ID: %u\n", table->table_id);
    }

    // Recorrer todas las entradas
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = get_page_table_entry(table, i);
        
        if (entry) {
            // Imprimir la información de la entrada
            print_page_table_entry(entry, level + 1, i);
            
            // Si no es el último nivel, imprimir recursivamente la siguiente tabla
            if (!entry->is_last_level && entry->next_table) {
                int new_path[path_len + 1];
                memcpy(new_path, path, path_len * sizeof(int));
                new_path[path_len] = i;
                
                print_page_table_recursive(entry->next_table, level + 1, new_path, path_len + 1);
            }
        }
    }
}

// Función para imprimir el estado completo de las tablas de páginas
void print_page_table(t_page_table* table) {
    int path[10] = {0}; // Un array vacío para el path inicial
    printf("\nEstructura completa de tablas de páginas:\n");
    print_page_table_recursive(table, 0, path, 0);
}

// Estructura para nuestra queue de BFS
typedef struct {
    t_page_table *table;
    int level;
} queue_item_t;

// Función auxiliar para recopilar todas las tablas de último nivel usando BFS
static void collect_last_level_tables(t_page_table *root_table, stack_item_t *result, int *result_size, int max_result_size) {
    // Crear una queue dinámica para BFS usando commons
    t_queue *bfs_queue = queue_create();
    
    // Crear el primer elemento para la cola
    queue_item_t *root_item = malloc(sizeof(queue_item_t));
    root_item->table = root_table;
    root_item->level = 0;
    
    // Agregar la tabla raíz a la queue
    queue_push(bfs_queue, root_item);
    
    // Mientras la queue no esté vacía
    while (!queue_is_empty(bfs_queue)) {
        // Sacar un elemento de la queue
        queue_item_t *current_item = queue_pop(bfs_queue);
        t_page_table *current_table = current_item->table;
        int current_level = current_item->level;
        
        // Si es una tabla de último nivel, la agregamos al resultado
        if (current_level == memoria_config.CANTIDAD_NIVELES - 1) {
            if (*result_size < max_result_size) {
                result[*result_size].table = current_table;
                result[*result_size].level = current_level;
                result[*result_size].entry_index = 0;
                (*result_size)++;
            }
            free(current_item);
            continue;
        }
        
        // Si no es una tabla de último nivel, agregamos sus hijos a la queue
        for (int i = 0; i < current_table->num_entries; i++) {
            t_page_table_entry *entry = simple_list_get(current_table->entries, i);
            if (entry && entry->next_table) {
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
bool assign_frames_to_process(t_page_table *root_table, void** frames_for_process, int pages_needed)
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
    for (int i = 0; i < num_last_level_tables && frame_index < pages_needed; i++) {
        t_page_table *last_level_table = last_level_tables[i].table;
        
        for (int j = 0; j < last_level_table->num_entries && frame_index < pages_needed; j++) {
            t_page_table_entry *entry = simple_list_get(last_level_table->entries, j);
            if (entry) {
                // Asignamos el frame
                uint32_t *frame_number = frames_for_process[frame_index]; // Acceso directo al arreglo
                if (frame_number) {
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

// Función para crear una lista simulada de frames
void** create_frame_list(int num_frames) {
    void** frames = safe_malloc(sizeof(void*) * num_frames);
    
    // Inicializar todos los elementos a NULL
    for (int i = 0; i < num_frames; i++) {
        frames[i] = NULL;
    }
    
    // Asignar números de frame
    for (int i = 0; i < num_frames; i++) {
        uint32_t* frame_num = safe_malloc(sizeof(uint32_t));
        *frame_num = 100 + i; // Asignar frames desde 100 para diferenciarlos fácilmente
        frames[i] = frame_num;
    }
    
    return frames;
}

// Función para contar cuántas entradas de último nivel tienen frames asignados
int count_assigned_frames(t_page_table* table, int current_level) {
    int count = 0;
    bool is_last_level = (current_level == memoria_config.CANTIDAD_NIVELES - 1);
    
    if (is_last_level) {
        // Estamos en el último nivel, contar entradas con frames asignados
        for (uint32_t i = 0; i < table->num_entries; i++) {
            t_page_table_entry* entry = simple_list_get(table->entries, i);
            if (entry && entry->frame_number != INVALID_FRAME_NUMBER) {
                count++;
            }
        }
    } else {
        // No estamos en el último nivel, recorrer recursivamente
        for (uint32_t i = 0; i < table->num_entries; i++) {
            t_page_table_entry* entry = simple_list_get(table->entries, i);
            if (entry && entry->next_table) {
                count += count_assigned_frames(entry->next_table, current_level + 1);
            }
        }
    }
    
    return count;
}

// Función para liberar la memoria de la lista de frames
void free_frame_list(void** frames, int num_frames) {
    for (int i = 0; i < num_frames; i++) {
        if (frames[i]) {
            free(frames[i]);
        }
    }
    free(frames);
}

// Test básico: asignar frames a un proceso
void test_assign_frames_basic() {
    printf("\n=== Test Básico de Asignación de Frames (Versión Iterativa) ===\n");
    
    // Crear una tabla de páginas
    t_page_table* root_table = init_page_table();
    printf("Tabla de páginas creada con %d niveles y %d entradas por tabla\n", 
           memoria_config.CANTIDAD_NIVELES, 
           memoria_config.ENTRADAS_POR_TABLA);
    
    // Contar cuántas páginas de último nivel hay en total
    int total_last_level_entries = pow(memoria_config.ENTRADAS_POR_TABLA, memoria_config.CANTIDAD_NIVELES);
    printf("Total de entradas de último nivel: %d\n", total_last_level_entries);
    
    // Probar con un número pequeño de frames para ver si se asignan correctamente
    int pages_needed = 5;
    printf("Asignando %d frames al proceso...\n", pages_needed);
    
    // Crear lista de frames
    void** frames = create_frame_list(pages_needed);
    
    // Asignar frames al proceso
    bool result = assign_frames_to_process(root_table, frames, pages_needed);
    
    printf("Resultado de la asignación: %s\n", result ? "ÉXITO" : "ERROR");
    
    // Verificar cuántos frames fueron asignados
    int assigned_frames = count_assigned_frames(root_table, 0);
    printf("Frames asignados: %d (esperados: %d)\n", assigned_frames, pages_needed);
    
    // Imprimir la estructura de las tablas después de la asignación
    printf("\nEstructura después de asignar frames:\n");
    print_page_table(root_table);
    
    // Liberar memoria
    free_frame_list(frames, pages_needed);
    free_page_table(root_table);
}

// Test con cantidad específica: asignar exactamente el número de frames que caben en la primera tabla de último nivel
void test_assign_frames_specific_amount() {
    printf("\n=== Test con Cantidad Específica (Versión Iterativa) ===\n");
    
    // Crear una tabla de páginas
    t_page_table* root_table = init_page_table();
    
    // Probar con exactamente el número de entradas en una tabla de último nivel
    int pages_needed = memoria_config.ENTRADAS_POR_TABLA;
    printf("Asignando exactamente %d frames (tamaño de una tabla de último nivel)...\n", pages_needed);
    
    // Crear lista de frames
    void** frames = create_frame_list(pages_needed);
    
    // Asignar frames al proceso
    bool result = assign_frames_to_process(root_table, frames, pages_needed);
    
    printf("Resultado de la asignación: %s\n", result ? "ÉXITO" : "ERROR");
    
    // Verificar cuántos frames fueron asignados
    int assigned_frames = count_assigned_frames(root_table, 0);
    printf("Frames asignados: %d (esperados: %d)\n", assigned_frames, pages_needed);
    
    // Imprimir la estructura de las tablas después de la asignación
    printf("\nEstructura después de asignar frames:\n");
    print_page_table(root_table);
    
    // Liberar memoria
    free_frame_list(frames, pages_needed);
    free_page_table(root_table);
}

// Test atravesando niveles: asignar frames que requieran pasar a la siguiente tabla
void test_assign_frames_across_tables() {
    printf("\n=== Test Atravesando Tablas (Versión Iterativa) ===\n");
    
    // Crear una tabla de páginas
    t_page_table* root_table = init_page_table();
    
    // Probar con más frames de los que caben en una tabla de último nivel
    int pages_needed = memoria_config.ENTRADAS_POR_TABLA + 2;
    printf("Asignando %d frames (más de los que caben en una tabla)...\n", pages_needed);
    
    // Crear lista de frames
    void** frames = create_frame_list(pages_needed);
    
    // Asignar frames al proceso
    bool result = assign_frames_to_process(root_table, frames, pages_needed);
    
    printf("Resultado de la asignación: %s\n", result ? "ÉXITO" : "ERROR");
    
    // Verificar cuántos frames fueron asignados
    int assigned_frames = count_assigned_frames(root_table, 0);
    printf("Frames asignados: %d (esperados: %d)\n", assigned_frames, pages_needed);
    
    // Imprimir la estructura de las tablas después de la asignación
    printf("\nEstructura después de asignar frames:\n");
    print_page_table(root_table);
    
    // Liberar memoria
    free_frame_list(frames, pages_needed);
    free_page_table(root_table);
}

// Test de caso límite: asignar más frames de los que caben en todas las tablas
void test_assign_frames_edge_case() {
    printf("\n=== Test de Caso Límite (Versión Iterativa) ===\n");
    
    // Crear una tabla de páginas pequeña para forzar un caso límite
    t_memoria_config backup = memoria_config;
    memoria_config.ENTRADAS_POR_TABLA = 2; // Reducir para el test
    
    t_page_table* root_table = init_page_table();
    printf("Tabla de páginas creada con %d niveles y %d entradas por tabla\n", 
           memoria_config.CANTIDAD_NIVELES, 
           memoria_config.ENTRADAS_POR_TABLA);
    
    // Calcular el número total de entradas de último nivel
    int total_last_level_entries = pow(memoria_config.ENTRADAS_POR_TABLA, memoria_config.CANTIDAD_NIVELES);
    printf("Total de entradas de último nivel: %d\n", total_last_level_entries);
    
    // Intentar asignar más frames de los que pueden caber
    int pages_needed = total_last_level_entries + 2;
    printf("Intentando asignar %d frames (más de los que caben)...\n", pages_needed);
    
    // Crear lista de frames
    void** frames = create_frame_list(pages_needed);
    
    // Asignar frames al proceso (debería asignar solo hasta llenar todas las entradas)
    bool result = assign_frames_to_process(root_table, frames, pages_needed);
    
    // Verificar cuántos frames fueron asignados
    int assigned_frames = count_assigned_frames(root_table, 0);
    printf("Resultado de la asignación: %s\n", result ? "ÉXITO" : "ERROR");
    printf("Frames asignados: %d (máximo posible: %d)\n", assigned_frames, total_last_level_entries);
    
    // Imprimir la estructura
    printf("\nEstructura después de intentar asignar demasiados frames:\n");
    print_page_table(root_table);
    
    // Restaurar la configuración original
    memoria_config = backup;
    
    // Liberar memoria
    free_frame_list(frames, pages_needed);
    free_page_table(root_table);
}

// Test de validación de parámetros
void test_assign_frames_validation() {
    printf("\n=== Test de Validación de Parámetros (Versión Iterativa) ===\n");
    
    // Caso 1: tabla de páginas nula
    printf("Caso 1: Tabla de páginas NULL\n");
    int pages_needed = 5;
    void** frames = create_frame_list(pages_needed);
    bool result = assign_frames_to_process(NULL, frames, pages_needed);
    printf("Resultado (esperado: false): %s\n", result ? "true" : "false");
    
    // Caso 2: lista de frames nula
    printf("\nCaso 2: Lista de frames NULL\n");
    t_page_table* root_table = init_page_table();
    result = assign_frames_to_process(root_table, NULL, pages_needed);
    printf("Resultado (esperado: false): %s\n", result ? "true" : "false");
    
    // Caso 3: número de páginas inválido
    printf("\nCaso 3: Número de páginas <= 0\n");
    result = assign_frames_to_process(root_table, frames, 0);
    printf("Resultado (esperado: false): %s\n", result ? "true" : "false");
    
    // Caso 4: número de páginas negativo
    printf("\nCaso 4: Número de páginas negativo\n");
    result = assign_frames_to_process(root_table, frames, -5);
    printf("Resultado (esperado: false): %s\n", result ? "true" : "false");
    
    // Liberar memoria
    free_frame_list(frames, pages_needed);
    free_page_table(root_table);
}

// Test de rendimiento (opcional): comparar el tiempo de ejecución en estructuras grandes
void test_performance() {
    printf("\n=== Test de Rendimiento (Versión Iterativa) ===\n");
    
    // Guardar configuración original
    t_memoria_config backup = memoria_config;
    
    // Configurar una estructura grande (ajustar según sea necesario)
    memoria_config.ENTRADAS_POR_TABLA = 10;
    memoria_config.CANTIDAD_NIVELES = 3;
    
    printf("Creando estructura grande con %d niveles y %d entradas por tabla...\n", 
           memoria_config.CANTIDAD_NIVELES, 
           memoria_config.ENTRADAS_POR_TABLA);
    
    t_page_table* root_table = init_page_table();
    
    // Calcular el total de entradas de último nivel
    int total_entries = pow(memoria_config.ENTRADAS_POR_TABLA, memoria_config.CANTIDAD_NIVELES);
    printf("Total de entradas de último nivel: %d\n", total_entries);
    
    // Asignar una cantidad moderada de frames
    int pages_needed = 50; // Ajustar según sea necesario
    printf("Asignando %d frames...\n", pages_needed);
    
    void** frames = create_frame_list(pages_needed);
    
    // Medir tiempo aproximado (muy básico)
    clock_t start = clock();
    bool result = assign_frames_to_process(root_table, frames, pages_needed);
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    int assigned_frames = count_assigned_frames(root_table, 0);
    printf("Resultado: %s\n", result ? "ÉXITO" : "ERROR");
    printf("Frames asignados: %d de %d\n", assigned_frames, pages_needed);
    printf("Tiempo aproximado: %f segundos\n", cpu_time_used);
    
    // Restaurar configuración
    memoria_config = backup;
    
    // Liberar memoria
    free_frame_list(frames, pages_needed);
    free_page_table(root_table);
}

int main() {
    printf("===== Test de Asignación de Frames a Proceso (Versión Iterativa) =====\n");
    
    // Ejecutar los tests
    test_assign_frames_basic();
    test_assign_frames_specific_amount();
    test_assign_frames_across_tables();
    test_assign_frames_edge_case();
    test_assign_frames_validation();
    
    // Test opcional de rendimiento (descomentar si se desea ejecutar)
    // test_performance();
    
    printf("\nTodos los tests completados.\n");
    
    return 0;
}
