#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

// Definiciones básicas para el test
#define INVALID_FRAME_NUMBER -1

// Estructura simulada de package y buffer para los tests
typedef struct {
    void* data;
    size_t size;
    size_t offset;
} t_buffer;

typedef struct {
    int operation_code;
    t_buffer* buffer;
} t_package;

// Estructura para los datos de solicitud de entrada de página
typedef struct {
    uint32_t pid;
    uint32_t table_id;
    uint32_t entry_index;
} page_entry_request_data;

// Estructura para los datos de respuesta de entrada de página
typedef struct {
    uint32_t value; // table_ptr o frame_number
    bool is_last_level;
} page_entry_response_data;

// Estructura de configuración de memoria
typedef struct {
    uint32_t TAM_PAGINA;
    uint32_t ENTRADAS_POR_TABLA;
    uint32_t CANTIDAD_NIVELES;
} t_memoria_config;

// Estructura para métricas de proceso
typedef struct {
    uint32_t page_table_access_count;
    uint32_t instruction_requests_count;
    uint32_t swap_out_count;
    uint32_t swap_in_count;
    uint32_t memory_read_count;
    uint32_t memory_write_count;
} t_process_metrics;

// Estructura para la tabla de páginas
typedef struct page_table {
    uint32_t table_id;
    size_t num_entries;
    void** entries;  // Simular una lista simple
} t_page_table;

// Estructura para una entrada de tabla de páginas
typedef struct page_table_entry {
    bool is_last_level;
    uint32_t next_table_id;
    union {
        struct page_table* next_table;  // Para niveles no-finales
        int32_t frame_number;           // Para el último nivel
    };
} t_page_table_entry;

// Estructura para la información de proceso
typedef struct {
    uint32_t pid;
    uint32_t process_size;
    bool is_suspended;
    void* instructions;  // No lo usaremos en el test
    t_page_table* page_table;
    t_process_metrics* metrics;
    void* allocated_frames;  // No lo usaremos en el test
} process_info;

// Configuración global simulada para las pruebas
t_memoria_config memoria_config = {
    .TAM_PAGINA = 256,
    .ENTRADAS_POR_TABLA = 4,  // Valor pequeño para facilitar las pruebas
    .CANTIDAD_NIVELES = 3     // 3 niveles jerárquicos para la prueba
};

// Variable estática para generar IDs únicos para las tablas de páginas
static uint32_t next_table_id = 0;
static process_info* test_process = NULL;

// Simulación de funciones de logger para los tests
#define LOG_DEBUG(format, ...) printf("[DEBUG] " format "\n", ##__VA_ARGS__)
#define LOG_INFO(format, ...) printf("[INFO] " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf("[ERROR] " format "\n", ##__VA_ARGS__)
#define LOG_OBLIGATORIO(format, ...) printf("[OBLIG] " format "\n", ##__VA_ARGS__)

// Funciones auxiliares para el test
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Funciones simplificadas para manejar listas
void** list_create() {
    void** list = (void**)safe_malloc(sizeof(void*) * memoria_config.ENTRADAS_POR_TABLA);
    memset(list, 0, sizeof(void*) * memoria_config.ENTRADAS_POR_TABLA);
    return list;
}

void list_add(void** list, void* element) {
    for (uint32_t i = 0; i < memoria_config.ENTRADAS_POR_TABLA; i++) {
        if (list[i] == NULL) {
            list[i] = element;
            return;
        }
    }
}

void* list_get(void** list, size_t index) {
    if (index >= memoria_config.ENTRADAS_POR_TABLA) {
        return NULL;
    }
    return list[index];
}

int list_size(void** list) {
    int count = 0;
    for (uint32_t i = 0; i < memoria_config.ENTRADAS_POR_TABLA; i++) {
        if (list[i] != NULL) {
            count++;
        }
    }
    return count;
}

void list_destroy(void** list) {
    free(list);
}

// Funciones para crear tablas de páginas
static t_page_table_entry* init_page_table_entry(bool is_last_level) {
    t_page_table_entry* entry = safe_malloc(sizeof(t_page_table_entry));
    
    entry->is_last_level = is_last_level;
    entry->next_table_id = 0;
    
    if (is_last_level) {
        entry->frame_number = INVALID_FRAME_NUMBER;
    } else {
        entry->next_table = NULL;
    }
    
    return entry;
}

static t_page_table* _create_table(int current_level) {
    t_page_table* table = safe_malloc(sizeof(t_page_table));
    
    table->table_id = next_table_id++;
    table->num_entries = memoria_config.ENTRADAS_POR_TABLA;
    table->entries = list_create();
    
    bool is_last_level = (current_level == memoria_config.CANTIDAD_NIVELES - 1);
    
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = init_page_table_entry(is_last_level);
        
        if (!is_last_level) {
            entry->next_table = _create_table(current_level + 1);
            entry->next_table_id = entry->next_table->table_id;
        }
        
        list_add(table->entries, entry);
    }
    
    return table;
}

t_page_table* init_page_table() {
    return _create_table(0);
}

// Liberación de memoria
void free_page_table(t_page_table* table) {
    if (!table) {
        return;
    }
    
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = list_get(table->entries, i);
        
        if (entry) {
            if (!entry->is_last_level && entry->next_table != NULL) {
                free_page_table(entry->next_table);
            }
            
            free(entry);
        }
    }
    
    list_destroy(table->entries);
    free(table);
}

// Función para encontrar una tabla por su ID (utilizada en handle_page_walk_request)
t_page_table* find_table_by_id(t_page_table* root_table, uint32_t target_id) {
    if (root_table == NULL) {
        return NULL;
    }

    if (root_table->table_id == target_id) {
        return root_table;
    }

    // Check if this is the last level
    if (list_size(root_table->entries) > 0) {
        t_page_table_entry* first_entry = list_get(root_table->entries, 0);
        if (first_entry->is_last_level) {
            // This is the last level, so we can't search deeper
            return NULL;
        }
    }

    // Recursively search all next-level tables
    for (int i = 0; i < root_table->num_entries; i++) {
        t_page_table_entry* entry = list_get(root_table->entries, i);
        if (!entry->is_last_level && entry->next_table != NULL) {
            t_page_table* found = find_table_by_id(entry->next_table, target_id);
            if (found != NULL) {
                return found;
            }
        }
    }

    return NULL;
}

// Asignar frames manualmente para el testing
void assign_specific_frames(t_page_table* table, uint32_t level, uint32_t* path, uint32_t path_len, int32_t frame_number) {
    if (level >= path_len) {
        return;
    }
    
    uint32_t index = path[level];
    t_page_table_entry* entry = list_get(table->entries, index);
    
    if (!entry) {
        printf("Error: No existe la entrada %u en la tabla ID %u\n", index, table->table_id);
        return;
    }
    
    if (level == path_len - 1) {
        if (entry->is_last_level) {
            entry->frame_number = frame_number;
            printf("Frame %d asignado a tabla %u, entrada %u\n", frame_number, table->table_id, index);
        } else {
            printf("Error: La entrada no es del último nivel\n");
        }
    } else {
        if (!entry->is_last_level && entry->next_table != NULL) {
            assign_specific_frames(entry->next_table, level + 1, path, path_len, frame_number);
        } else {
            printf("Error: La entrada no apunta a una tabla válida\n");
        }
    }
}

// Función para simular la función del módulo de memoria
process_info* process_manager_find_process(uint32_t pid) {
    if (test_process && test_process->pid == pid) {
        return test_process;
    }
    return NULL;
}

// Función que simula el envío de respuesta
void send_page_entry_response_package(int client_socket, uint32_t value, uint32_t is_last_level) {
    printf("Enviando respuesta: value=%u, is_last_level=%u\n", value, is_last_level);
}

// Implementación del handle_page_walk_request para el test
void handle_page_walk_request(int client_socket, page_entry_request_data payload) {
    // Find the process with the given PID
    process_info* process = process_manager_find_process(payload.pid);
    
    if (process == NULL) {
        // Process not found - sending error response
        LOG_ERROR("Process with PID %d not found", payload.pid);
        send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, true);
        return;
    }

    // Increment metrics for page table access
    process->metrics->page_table_access_count++;

    // Search for the table with the given ID
    t_page_table* current_table = process->page_table;
    t_page_table* target_table = NULL;

    // If we're looking for the first level table, it's already the process's page_table
    if (current_table->table_id == payload.table_id) {
        target_table = current_table;
    } else {
        // Otherwise we need to search through all tables recursively
        target_table = find_table_by_id(process->page_table, payload.table_id);
    }

    if (target_table == NULL) {
        LOG_ERROR("Table with ID %d not found for process %d", payload.table_id, payload.pid);
        send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, true);
        return;
    }

    // Check if the entry_index is valid
    if (payload.entry_index >= target_table->num_entries) {
        LOG_ERROR("Entry index %u out of bounds for table %u (max: %zu)", 
                payload.entry_index, payload.table_id, target_table->num_entries - 1);
        send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, true);
        return;
    }

    // Get the entry at the specified index
    t_page_table_entry* entry = list_get(target_table->entries, payload.entry_index);
    
    // Determine response value based on whether this is the last level or not
    uint32_t response_value;
    if (entry->is_last_level) {
        // This is the last level, so we return the frame number
        response_value = entry->frame_number;
        LOG_DEBUG("Returning frame number %d for PID %d, table %d, entry %d", 
                response_value, payload.pid, payload.table_id, payload.entry_index);
    } else {
        // This is an intermediate level, so we return the next table ID
        response_value = entry->next_table_id;
        LOG_DEBUG("Returning next table ID %d for PID %d, table %d, entry %d", 
                response_value, payload.pid, payload.table_id, payload.entry_index);
    }

    // Send the response back to the CPU
    send_page_entry_response_package(client_socket, response_value, entry->is_last_level);
}

// Función para imprimir una estructura de tablas de páginas
void print_page_table_structure(t_page_table* table, int level, int path[], int path_len) {
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
        t_page_table_entry* entry = list_get(table->entries, i);
        
        if (entry) {
            for (int j = 0; j < level + 1; j++) {
                printf("  ");
            }
            
            if (entry->is_last_level) {
                printf("Entrada %u: Frame = %s (%d)\n", 
                       i, 
                       (entry->frame_number == INVALID_FRAME_NUMBER) ? "INVALID" : "ASIGNADO",
                       entry->frame_number);
            } else {
                printf("Entrada %u: Siguiente tabla ID = %u\n", i, entry->next_table_id);
                
                // Actualizar el path para la recursión
                int new_path[path_len + 1];
                memcpy(new_path, path, path_len * sizeof(int));
                new_path[path_len] = i;
                
                // Recursivamente imprimir la tabla siguiente
                print_page_table_structure(entry->next_table, level + 1, new_path, path_len + 1);
            }
        }
    }
}

// Función para crear un proceso de prueba
process_info* create_test_process(uint32_t pid) {
    process_info* proc = safe_malloc(sizeof(process_info));
    proc->pid = pid;
    proc->process_size = 1024; // 1KB por simplicidad
    proc->is_suspended = false;
    proc->instructions = NULL; // No es relevante para este test
    proc->page_table = init_page_table();
    proc->metrics = safe_malloc(sizeof(t_process_metrics));
    memset(proc->metrics, 0, sizeof(t_process_metrics));
    proc->allocated_frames = NULL; // No es relevante para este test
    
    return proc;
}

// Test case que prueba el acceso a una entrada de tabla de páginas de nivel intermedio
void test_intermediate_table_entry() {
    printf("\n=== Test: Acceso a una entrada de tabla de nivel intermedio ===\n");
    
    // Crear una solicitud para una tabla de nivel 1, entrada 2
    page_entry_request_data request = {
        .pid = 1,
        .table_id = 0, // ID de la tabla raíz
        .entry_index = 2 // Accedemos a la entrada 2
    };
    
    // Probar el manejo de la solicitud
    handle_page_walk_request(1, request);
    
    // Verificar el contador de accesos a la tabla de páginas
    assert(test_process->metrics->page_table_access_count == 1);
    
    printf("Test de acceso a tabla intermedia completado correctamente.\n");
}

// Test case que prueba el acceso a una entrada de tabla de páginas de último nivel
void test_last_level_table_entry() {
    printf("\n=== Test: Acceso a una entrada de tabla de último nivel ===\n");
    
    // Primero necesitamos obtener el ID de una tabla de último nivel
    t_page_table* root = test_process->page_table;
    t_page_table_entry* entry1 = list_get(root->entries, 1);
    t_page_table* level1_table = entry1->next_table;
    t_page_table_entry* entry2 = list_get(level1_table->entries, 3);
    t_page_table* level2_table = entry2->next_table;
    uint32_t last_level_table_id = level2_table->table_id;
    
    // Crear una solicitud para esa tabla de último nivel, entrada 0
    page_entry_request_data request = {
        .pid = 1,
        .table_id = last_level_table_id,
        .entry_index = 0
    };
    
    // Asignar un frame a esa entrada para el test
    t_page_table_entry* last_level_entry = list_get(level2_table->entries, 0);
    last_level_entry->frame_number = 42; // Asignamos un frame arbitrario
    
    // Probar el manejo de la solicitud
    handle_page_walk_request(1, request);
    
    // Verificar el contador de accesos a la tabla de páginas
    assert(test_process->metrics->page_table_access_count == 2);
    
    printf("Test de acceso a tabla de último nivel completado correctamente.\n");
}

// Test case que prueba el acceso a una tabla inexistente
void test_nonexistent_table() {
    printf("\n=== Test: Acceso a una tabla inexistente ===\n");
    
    // Crear una solicitud con un ID de tabla inválido
    page_entry_request_data request = {
        .pid = 1,
        .table_id = 999, // ID que no existe
        .entry_index = 0
    };
    
    // Probar el manejo de la solicitud
    handle_page_walk_request(1, request);
    
    // Verificar el contador de accesos a la tabla de páginas
    assert(test_process->metrics->page_table_access_count == 3);
    
    printf("Test de acceso a tabla inexistente completado correctamente.\n");
}

// Test case que prueba el acceso a un proceso inexistente
void test_nonexistent_process() {
    printf("\n=== Test: Acceso a un proceso inexistente ===\n");
    
    // Crear una solicitud con un PID inválido
    page_entry_request_data request = {
        .pid = 999, // PID que no existe
        .table_id = 0,
        .entry_index = 0
    };
    
    // Probar el manejo de la solicitud
    handle_page_walk_request(1, request);
    
    // El contador no debería cambiar ya que el proceso no existe
    assert(test_process->metrics->page_table_access_count == 3);
    
    printf("Test de acceso a proceso inexistente completado correctamente.\n");
}

// Test case que prueba el acceso a un índice de entrada inválido
void test_invalid_entry_index() {
    printf("\n=== Test: Acceso a un índice de entrada inválido ===\n");
    
    // Crear una solicitud con un índice fuera de rango
    page_entry_request_data request = {
        .pid = 1,
        .table_id = 0, // ID de la tabla raíz
        .entry_index = 10 // Índice mayor que el número de entradas por tabla
    };
    
    // Probar el manejo de la solicitud
    handle_page_walk_request(1, request);
    
    // Verificar el contador de accesos a la tabla de páginas
    assert(test_process->metrics->page_table_access_count == 4);
    
    printf("Test de acceso a índice inválido completado correctamente.\n");
}

// Función principal
int main() {
    printf("Iniciando tests de handle_page_walk_request\n");
    
    // Crear un proceso de prueba
    test_process = create_test_process(1);
    
    // Mostrar la estructura de las tablas de páginas
    printf("\n=== Estructura de tablas de páginas del proceso ===\n");
    int path[10] = {0}; // Array vacío para el path inicial
    print_page_table_structure(test_process->page_table, 0, path, 0);
    
    // Ejecutar los tests
    test_intermediate_table_entry();
    test_last_level_table_entry();
    test_nonexistent_table();
    test_nonexistent_process();
    test_invalid_entry_index();
    
    // Liberar memoria
    free_page_table(test_process->page_table);
    free(test_process->metrics);
    free(test_process);
    
    printf("\nTodos los tests completados correctamente!\n");
    return EXIT_SUCCESS;
}
