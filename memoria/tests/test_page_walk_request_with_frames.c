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
static uint32_t response_value_received = 0;
static bool is_last_level_received = false;

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

// Recolectar todas las tablas de último nivel y asignarles frames
void assign_frames_to_last_level_tables(t_page_table* table, int* frame_counter) {
    if (table == NULL) {
        return;
    }

    // Si es una tabla de último nivel, asignar frames a todas sus entradas
    if (list_size(table->entries) > 0) {
        t_page_table_entry* first_entry = list_get(table->entries, 0);
        if (first_entry->is_last_level) {
            for (uint32_t i = 0; i < table->num_entries; i++) {
                t_page_table_entry* entry = list_get(table->entries, i);
                entry->frame_number = (*frame_counter)++;
                printf("Asignado frame %d a tabla ID %u, entrada %u\n", 
                       entry->frame_number, table->table_id, i);
            }
            return;
        }
    }

    // Si no es tabla de último nivel, recursivamente procesar sus sub-tablas
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = list_get(table->entries, i);
        if (!entry->is_last_level && entry->next_table != NULL) {
            assign_frames_to_last_level_tables(entry->next_table, frame_counter);
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
    response_value_received = value;
    is_last_level_received = is_last_level ? true : false;
}

// Implementación del handle_page_walk_request para el test
void handle_page_walk_request(int client_socket, page_entry_request_data payload) {
    // Find the process with the given PID
    process_info* process = process_manager_find_process(payload.pid);
    
    if (process == NULL) {
        // Process not found - sending error response
        LOG_ERROR("Process with PID %d not found", payload.pid);
        send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, 1);
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
        send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, 1);
        return;
    }

    // Check if the entry_index is valid
    if (payload.entry_index >= target_table->num_entries) {
        LOG_ERROR("Entry index %u out of bounds for table %u (max: %zu)", 
                payload.entry_index, payload.table_id, target_table->num_entries - 1);
        send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, 1);
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

    // Send the response back to the CPU - is_last_level es 1 si es último nivel, 0 si no
    send_page_entry_response_package(client_socket, response_value, entry->is_last_level ? 1 : 0);
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

// Simula una serie de peticiones de CPU para encontrar el frame para la página virtual 5
void simulate_cpu_page_walk_for_frame(uint32_t virtual_page) {
    printf("\n=== Simulando page walk de CPU para página virtual %u ===\n", virtual_page);
    
    // Para este ejemplo simple, asumimos que:
    // - Nivel 1 usa los bits más significativos
    // - Nivel 2 usa los bits del medio
    // - Nivel 3 usa los bits menos significativos
    
    // Calcular los índices para cada nivel según el tamaño de página y entradas por tabla
    int bits_por_nivel = 0;
    int temp = memoria_config.ENTRADAS_POR_TABLA;
    while (temp > 1) {
        temp >>= 1;
        bits_por_nivel++;
    }
    
    uint32_t mask = (1 << bits_por_nivel) - 1;
    
    // Calcular índices para cada nivel
    uint32_t indices[3];
    indices[2] = virtual_page & mask;
    indices[1] = (virtual_page >> bits_por_nivel) & mask;
    indices[0] = (virtual_page >> (bits_por_nivel * 2)) & mask;
    
    printf("Índices calculados: Nivel 1: %u, Nivel 2: %u, Nivel 3: %u\n", indices[0], indices[1], indices[2]);
    
    // Iniciar el proceso desde la tabla de primer nivel
    uint32_t current_table_id = test_process->page_table->table_id;
    
    // Primer nivel
    page_entry_request_data request1 = {
        .pid = 1,
        .table_id = current_table_id,
        .entry_index = indices[0]
    };
    
    printf("\nPASO 1: Accediendo a tabla nivel 1 (ID: %u), entrada %u\n", current_table_id, indices[0]);
    handle_page_walk_request(1, request1);
    
    // Verificar que obtuvimos un ID de tabla y no un frame
    assert(is_last_level_received == false);
    current_table_id = response_value_received;
    
    // Segundo nivel
    page_entry_request_data request2 = {
        .pid = 1,
        .table_id = current_table_id,
        .entry_index = indices[1]
    };
    
    printf("\nPASO 2: Accediendo a tabla nivel 2 (ID: %u), entrada %u\n", current_table_id, indices[1]);
    handle_page_walk_request(1, request2);
    
    // Verificar que obtuvimos un ID de tabla y no un frame
    assert(is_last_level_received == false);
    current_table_id = response_value_received;
    
    // Tercer nivel (último)
    page_entry_request_data request3 = {
        .pid = 1,
        .table_id = current_table_id,
        .entry_index = indices[2]
    };
    
    printf("\nPASO 3: Accediendo a tabla nivel 3 (ID: %u), entrada %u\n", current_table_id, indices[2]);
    handle_page_walk_request(1, request3);
    
    // Verificar que obtuvimos un frame (último nivel)
    assert(is_last_level_received == true);
    uint32_t frame_number = response_value_received;
    
    printf("\n=== Resultado del page walk ===\n");
    printf("Página virtual %u -> Frame físico %u\n", virtual_page, frame_number);
}

// Función principal
int main() {
    printf("Iniciando tests de handle_page_walk_request con asignación de frames\n");
    
    // Crear un proceso de prueba
    test_process = create_test_process(1);
    
    // Asignar frames a todas las tablas de último nivel
    int frame_counter = 1; // Empezamos desde 1 para evitar confusión con INVALID_FRAME_NUMBER
    assign_frames_to_last_level_tables(test_process->page_table, &frame_counter);
    
    // Mostrar la estructura de las tablas de páginas con frames asignados
    printf("\n=== Estructura de tablas de páginas del proceso CON FRAMES ASIGNADOS ===\n");
    int path[10] = {0}; // Array vacío para el path inicial
    print_page_table_structure(test_process->page_table, 0, path, 0);
    
    // Simular varios page walks para diferentes páginas virtuales
    simulate_cpu_page_walk_for_frame(5);  // Página virtual 5
    simulate_cpu_page_walk_for_frame(10); // Página virtual 10
    simulate_cpu_page_walk_for_frame(15); // Página virtual 15
    
    // Liberar memoria
    free_page_table(test_process->page_table);
    free(test_process->metrics);
    free(test_process);
    
    printf("\nTodos los tests completados correctamente!\n");
    return EXIT_SUCCESS;
}
