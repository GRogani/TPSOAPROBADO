#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

// Funciones simplificadas para manejar listas
void** list_create() {
    return (void**)safe_malloc(sizeof(void*) * memoria_config.ENTRADAS_POR_TABLA);
}

void list_add(void** list, void* element) {
    // Buscamos la primera posición vacía y añadimos el elemento
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

void list_destroy(void** list) {
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
    table->entries = list_create();
    
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
        
        list_add(table->entries, entry);
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
    
    return list_get(table->entries, index);
}

// Función para liberar la memoria de una tabla de páginas
void free_page_table(t_page_table* table) {
    if (!table) {
        return;
    }
    
    // Liberar todas las entradas
    for (uint32_t i = 0; i < table->num_entries; i++) {
        t_page_table_entry* entry = list_get(table->entries, i);
        
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
    list_destroy(table->entries);
    free(table);
}

// Función para imprimir el estado de las tablas de páginas (ayuda a visualizar)
void print_page_table(t_page_table* table, int level, int index_path[], int path_len) {
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
            printf("%d", index_path[i]);
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
            for (int j = 0; j < level + 1; j++) {
                printf("  ");
            }
            
            if (entry->is_last_level) {
                printf("Entrada %u: Frame = %s\n", 
                       i, 
                       (entry->frame_number == INVALID_FRAME_NUMBER) ? "INVALID" : "ASIGNADO");
            } else {
                printf("Entrada %u: Siguiente tabla ID = %u\n", i, entry->next_table_id);
                
                // Actualizar el path para la recursión
                int new_path[path_len + 1];
                memcpy(new_path, index_path, path_len * sizeof(int));
                new_path[path_len] = i;
                
                // Recursivamente imprimir la tabla siguiente
                print_page_table(entry->next_table, level + 1, new_path, path_len + 1);
            }
        }
    }
}

// Test de la creación de tablas de páginas
void test_page_table_creation() {
    printf("=== Test de creación de tablas de páginas ===\n");
    
    // Crear una tabla de páginas
    printf("Creando tabla de páginas con %d niveles y %d entradas por tabla...\n", 
           memoria_config.CANTIDAD_NIVELES, 
           memoria_config.ENTRADAS_POR_TABLA);
    
    t_page_table* root_table = init_page_table();
    
    // Verificar que la tabla raíz se creó correctamente
    printf("Tabla raíz creada con ID: %u\n", root_table->table_id);
    
    // Imprimir la estructura completa
    int path[10] = {0}; // Un array vacío para el path inicial
    printf("\nEstructura completa de tablas de páginas:\n");
    print_page_table(root_table, 0, path, 0);
    
    // Contar tablas y entradas
    uint32_t num_tables = next_table_id;
    printf("\nEstadísticas:\n");
    printf("- Número total de tablas creadas: %u\n", num_tables);
    printf("- Número de entradas por tabla: %u\n", memoria_config.ENTRADAS_POR_TABLA);
    printf("- Niveles configurados: %u\n", memoria_config.CANTIDAD_NIVELES);
    
    // Liberar la memoria
    printf("\nLiberando memoria...\n");
    free_page_table(root_table);
    printf("Memoria liberada.\n");
}

// Función principal
int main() {
    test_page_table_creation();
    
    printf("\n=== Probando con otra configuración ===\n");
    // Reiniciar el contador de IDs para la próxima prueba
    next_table_id = 0;
    
    // Cambiar la configuración para otra prueba
    memoria_config.CANTIDAD_NIVELES = 2;
    memoria_config.ENTRADAS_POR_TABLA = 3;
    
    test_page_table_creation();
    
    return EXIT_SUCCESS;
}
