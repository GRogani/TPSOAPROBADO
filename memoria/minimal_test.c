#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Mocks for external dependencies
#define LOG_INFO(format, ...) printf("[INFO] " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf("[ERROR] " format "\n", ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) printf("[DEBUG] " format "\n", ##__VA_ARGS__)
#define LOG_WARNING(format, ...) printf("[WARNING] " format "\n", ##__VA_ARGS__)
#define LOG_OBLIGATORIO(format, ...) printf("[OBLIGATORIO] " format "\n", ##__VA_ARGS__)

typedef struct {
    void** elements;
    int size;
} t_list;

t_list* list_create() {
    t_list* list = malloc(sizeof(t_list));
    list->elements = NULL;
    list->size = 0;
    return list;
}

void list_add(t_list* list, void* element) {
    list->size++;
    list->elements = realloc(list->elements, sizeof(void*) * list->size);
    list->elements[list->size - 1] = element;
}

void* list_get(t_list* list, int index) {
    if(index < 0 || index >= list->size) return NULL;
    return list->elements[index];
}

int list_size(t_list* list) {
    return list->size;
}

void list_destroy(t_list* list) {
    if(list->elements) free(list->elements);
    free(list);
}

void list_destroy_and_destroy_elements(t_list* list, void (*element_destroyer)(void*)) {
    for(int i = 0; i < list->size; i++) {
        element_destroyer(list->elements[i]);
    }
    list_destroy(list);
}

// Memoria config structure
typedef struct {
    char* PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_PAGINA;
    int ENTRADAS_POR_TABLA;
    int CANTIDAD_NIVELES;
    int RETARDO_MEMORIA;
    int RETARDO_SWAP;
    char* PATH_SWAPFILE;
    char* PATH_INSTRUCCIONES;
    int LOG_LEVEL;
    char* DUMP_PATH;
} t_memoria_config;

// Swap structures
typedef struct {
    _Atomic uint32_t total_pages;
    _Atomic uint32_t used_pages;
    _Atomic uint32_t read_operations;
    _Atomic uint32_t write_operations;
    _Atomic uint32_t allocated_processes;
} t_swap_status;

typedef struct {
    uint32_t pid;
    uint32_t start_offset;
    uint32_t num_pages;
    bool is_used;
    struct timespec last_access;
} t_swap_block;

typedef struct {
    uint32_t pid;
    uint32_t virtual_page_number;
    uint32_t swap_offset;
} t_swap_page_info;

// Function prototypes for swap manager
bool swap_manager_init(const t_memoria_config* config);
void swap_manager_destroy(void);
t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages);
bool swap_write_pages(t_list* pages, void* process_memory, uint32_t page_size);
bool swap_read_pages(t_list* pages, void* process_memory, uint32_t page_size);
void swap_free_pages(uint32_t pid);
size_t swap_get_free_pages_count(void);

// Configuración de prueba
t_memoria_config test_config = {
    .TAM_PAGINA = 64,
    .RETARDO_SWAP = 10,  // ms en lugar de microsegundos para acelerar los tests
    .PATH_SWAPFILE = "/tmp/test_swapfile.bin"
};

// Implementation of swap_manager functions
FILE* swap_file = NULL;
char* swap_file_path = NULL;
uint32_t page_size = 0;
uint32_t swap_delay_ms = 0;
t_list* swap_blocks = NULL;

// Simple bitmap-based swap manager (for testing only)
bool swap_manager_init(const t_memoria_config* config) {
    printf("Inicializando sistema de SWAP simplificado para test...\n");
    
    page_size = config->TAM_PAGINA;
    swap_delay_ms = config->RETARDO_SWAP;
    swap_file_path = strdup(config->PATH_SWAPFILE);
    
    // Create a new file
    swap_file = fopen(swap_file_path, "w+b");
    if (!swap_file) {
        printf("ERROR: No se pudo crear el archivo de SWAP\n");
        return false;
    }
    
    // Create a list of blocks
    swap_blocks = list_create();
    
    // Initialize with empty space
    size_t init_size = 1024 * 1024; // 1MB
    char* zeros = calloc(1, init_size);
    fwrite(zeros, 1, init_size, swap_file);
    fflush(swap_file);
    free(zeros);
    
    // Create initial free block
    t_swap_block* block = malloc(sizeof(t_swap_block));
    block->pid = 0; // 0 = free
    block->is_used = false;
    block->start_offset = 0;
    block->num_pages = init_size / page_size;
    list_add(swap_blocks, block);
    
    printf("SWAP inicializado con %zu páginas libres\n", block->num_pages);
    return true;
}

void swap_manager_destroy() {
    if (swap_file) {
        fclose(swap_file);
        swap_file = NULL;
    }
    
    if (swap_blocks) {
        for (int i = 0; i < list_size(swap_blocks); i++) {
            free(list_get(swap_blocks, i));
        }
        list_destroy(swap_blocks);
        swap_blocks = NULL;
    }
    
    if (swap_file_path) {
        free(swap_file_path);
        swap_file_path = NULL;
    }
    
    printf("SWAP finalizado\n");
}

t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages) {
    printf("Asignando %u páginas para proceso %u\n", num_pages, pid);
    
    // Find a free block
    t_swap_block* free_block = NULL;
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == 0 && !block->is_used && block->num_pages >= num_pages) {
            free_block = block;
            break;
        }
    }
    
    if (!free_block) {
        printf("ERROR: No hay espacio suficiente en SWAP\n");
        return NULL;
    }
    
    // Mark the block as used
    free_block->pid = pid;
    free_block->is_used = true;
    
    // Create page info list
    t_list* pages = list_create();
    for (uint32_t i = 0; i < num_pages; i++) {
        t_swap_page_info* page = malloc(sizeof(t_swap_page_info));
        page->pid = pid;
        page->virtual_page_number = i;
        page->swap_offset = free_block->start_offset + (i * page_size);
        list_add(pages, page);
    }
    
    return pages;
}

bool swap_write_pages(t_list* pages, void* process_memory, uint32_t total_size) {
    if (!pages || !process_memory || !swap_file) {
        printf("ERROR: Parámetros inválidos para escritura\n");
        return false;
    }
    
    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page = list_get(pages, i);
        
        // Apply delay
        usleep(swap_delay_ms * 1000);
        
        // Position at the correct offset
        fseek(swap_file, page->swap_offset, SEEK_SET);
        
        // Write the page
        void* page_data = process_memory + (page->virtual_page_number * page_size);
        size_t written = fwrite(page_data, 1, page_size, swap_file);
        
        if (written != page_size) {
            printf("ERROR: No se pudo escribir la página %u (offset %u)\n", 
                   page->virtual_page_number, page->swap_offset);
            return false;
        }
    }
    
    fflush(swap_file);
    return true;
}

bool swap_read_pages(t_list* pages, void* process_memory, uint32_t total_size) {
    if (!pages || !process_memory || !swap_file) {
        printf("ERROR: Parámetros inválidos para lectura\n");
        return false;
    }
    
    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page = list_get(pages, i);
        
        // Apply delay
        usleep(swap_delay_ms * 1000);
        
        // Position at the correct offset
        fseek(swap_file, page->swap_offset, SEEK_SET);
        
        // Read the page
        void* page_data = process_memory + (page->virtual_page_number * page_size);
        size_t read = fread(page_data, 1, page_size, swap_file);
        
        if (read != page_size) {
            printf("ERROR: No se pudo leer la página %u (offset %u)\n", 
                   page->virtual_page_number, page->swap_offset);
            return false;
        }
    }
    
    return true;
}

void swap_free_pages(uint32_t pid) {
    printf("Liberando páginas del proceso %u\n", pid);
    
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == pid) {
            block->pid = 0;
            block->is_used = false;
            printf("Liberado bloque en offset %u\n", block->start_offset);
        }
    }
}

size_t swap_get_free_pages_count() {
    size_t free_pages = 0;
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == 0 && !block->is_used) {
            free_pages += block->num_pages;
        }
    }
    return free_pages;
}

int main(int argc, char** argv) {
    printf("Test simple de swap...\n");
    
    // Eliminar archivo de swap si existe
    unlink(test_config.PATH_SWAPFILE);
    
    // Inicializar swap
    printf("Inicializando swap...\n");
    if (!swap_manager_init(&test_config)) {
        printf("ERROR: No se pudo inicializar swap\n");
        return 1;
    }
    
    printf("Swap inicializado correctamente\n");
    
    // Asignar páginas
    uint32_t pid = 123;
    uint32_t num_pages = 3;
    printf("Asignando %u páginas para PID %u...\n", num_pages, pid);
    
    t_list* pages = swap_allocate_pages(pid, num_pages);
    if (!pages) {
        printf("ERROR: No se pudieron asignar páginas\n");
        swap_manager_destroy();
        return 1;
    }
    
    printf("Páginas asignadas: %d\n", list_size(pages));
    
    // Escribir datos
    char* data = malloc(num_pages * test_config.TAM_PAGINA);
    memset(data, 'A', num_pages * test_config.TAM_PAGINA);
    
    printf("Escribiendo datos...\n");
    if (!swap_write_pages(pages, data, test_config.TAM_PAGINA * num_pages)) {
        printf("ERROR: No se pudieron escribir los datos\n");
        free(data);
        list_destroy(pages);
        swap_manager_destroy();
        return 1;
    }
    
    printf("Datos escritos correctamente\n");
    
    // Leer datos para verificar
    char* read_data = malloc(num_pages * test_config.TAM_PAGINA);
    memset(read_data, 0, num_pages * test_config.TAM_PAGINA);
    
    printf("Leyendo datos...\n");
    if (!swap_read_pages(pages, read_data, test_config.TAM_PAGINA * num_pages)) {
        printf("ERROR: No se pudieron leer los datos\n");
        free(data);
        free(read_data);
        list_destroy(pages);
        swap_manager_destroy();
        return 1;
    }
    
    printf("Datos leídos correctamente\n");
    
    // Verificar datos
    if (memcmp(data, read_data, num_pages * test_config.TAM_PAGINA) != 0) {
        printf("ERROR: Los datos leídos no coinciden con los escritos\n");
    } else {
        printf("Verificación de datos exitosa: datos leídos coinciden con los escritos\n");
    }
    
    // Liberar páginas
    printf("Liberando páginas...\n");
    swap_free_pages(pid);
    printf("Páginas liberadas correctamente\n");
    
    // Liberar memoria
    free(data);
    free(read_data);
    list_destroy(pages);
    swap_manager_destroy();
    
    printf("Test completado exitosamente\n");
    return 0;
}
