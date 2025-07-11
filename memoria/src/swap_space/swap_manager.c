#include "swap_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE* swap_file = NULL;
static char* swap_file_path = NULL;
static uint32_t page_size = 0;
static uint32_t total_pages = 0;
static uint32_t next_free_offset = 0;

// Bitmap para gestionar espacios libres
static bool* free_pages_bitmap = NULL;
static uint32_t bitmap_size = 0;

// Lista de procesos en swap (para tracking)
static t_list* process_blocks = NULL;

/**
 * @brief Compara dos bloques de proceso por offset de inicio
 * @param a Primer bloque
 * @param b Segundo bloque
 * @return true si a < b, false en caso contrario
 */
bool compare_by_offset(void* a, void* b) {
    if (a == NULL || b == NULL) return false;
    t_swap_process_info* block_a = (t_swap_process_info*)a;
    t_swap_process_info* block_b = (t_swap_process_info*)b;
    return block_a->start_offset < block_b->start_offset;
}

/**
 * @brief Libera un elemento t_swap_process_info
 * @param element Puntero al elemento a liberar
 */
void free_swap_process_info(void* element) {
    if (element != NULL) {
        t_swap_process_info* info = (t_swap_process_info*)element;
        free(info);
    }
}

/**
 * @brief Busca espacio libre usando bitmap (First-Fit)
 * @param pages_needed Número de páginas necesarias
 * @return Offset de inicio del espacio libre o -1 si no hay
 */
uint32_t find_free_space_with_bitmap(uint32_t pages_needed) {
    if (free_pages_bitmap == NULL || pages_needed == 0) {
        return 0; // Archivo vacío o parámetro inválido
    }
    
    uint32_t consecutive_free = 0;
    uint32_t start_page = 0;
    
    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (free_pages_bitmap[i]) {
            // Página libre
            if (consecutive_free == 0) {
                start_page = i;
            }
            consecutive_free++;
            
            if (consecutive_free >= pages_needed) {
                return start_page * page_size;
            }
        } else {
            // Página ocupada, resetear contador
            consecutive_free = 0;
        }
    }
    
    return -1; // No hay espacio libre suficiente
}

/**
 * @brief Marca páginas como ocupadas en el bitmap
 * @param start_page Página de inicio
 * @param num_pages Número de páginas a marcar
 */
void mark_pages_as_used(uint32_t start_page, uint32_t num_pages) {
    if (free_pages_bitmap == NULL || num_pages == 0) {
        return;
    }
    
    for (uint32_t i = 0; i < num_pages && (start_page + i) < bitmap_size; i++) {
        free_pages_bitmap[start_page + i] = false;
    }
}

/**
 * @brief Marca páginas como libres en el bitmap
 * @param start_page Página de inicio
 * @param num_pages Número de páginas a marcar
 */
void mark_pages_as_free(uint32_t start_page, uint32_t num_pages) {
    if (free_pages_bitmap == NULL || num_pages == 0) {
        return;
    }
    
    for (uint32_t i = 0; i < num_pages && (start_page + i) < bitmap_size; i++) {
        free_pages_bitmap[start_page + i] = true;
    }
}

/**
 * @brief Extiende el bitmap si es necesario
 * @param new_pages_needed Número de páginas adicionales necesarias
 * @return true si éxito, false si error
 */
bool extend_bitmap_if_needed(uint32_t new_pages_needed) {
    if (new_pages_needed == 0) {
        return true;
    }
    
    uint32_t new_bitmap_size = bitmap_size + new_pages_needed;
    bool* new_bitmap = realloc(free_pages_bitmap, new_bitmap_size * sizeof(bool));
    
    if (new_bitmap == NULL) {
        LOG_ERROR("Swap Manager: Error extendiendo bitmap de %u a %u páginas", bitmap_size, new_bitmap_size);
        return false;
    }
    
    free_pages_bitmap = new_bitmap;
    // Marcar las nuevas páginas como libres
    for (uint32_t i = bitmap_size; i < new_bitmap_size; i++) {
        free_pages_bitmap[i] = true;
    }
    bitmap_size = new_bitmap_size;
    return true;
}

// --- Funciones Públicas del Swap Manager ---

bool swap_manager_init(const t_memoria_config* config) {
    if (config == NULL || config->TAM_PAGINA == 0 || config->PATH_SWAPFILE == NULL) {
        LOG_ERROR("Swap Manager: Configuracion invalida para inicializacion");
        return false;
    }

    // Limpiar estado previo si existe
    swap_manager_destroy();

    swap_file_path = strdup(config->PATH_SWAPFILE);
    if (swap_file_path == NULL) {
        LOG_ERROR("Swap Manager: Error al duplicar PATH_SWAPFILE");
        return false;
    }

    page_size = config->TAM_PAGINA;

    // Abrir o crear el archivo de swap
    swap_file = fopen(swap_file_path, "r+b");
    if (swap_file == NULL) {
        swap_file = fopen(swap_file_path, "w+b");
        if (swap_file == NULL) {
            LOG_ERROR("Swap Manager: No se pudo abrir/crear el archivo SWAP: %s", swap_file_path);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }
    }

    // Inicializar estructuras
    process_blocks = list_create();
    if (process_blocks == NULL) {
        LOG_ERROR("Swap Manager: Error al crear lista de bloques de proceso");
        fclose(swap_file);
        swap_file = NULL;
        free(swap_file_path);
        swap_file_path = NULL;
        return false;
    }

    // Obtener el tamaño actual del archivo y calcular páginas
    if (fseek(swap_file, 0, SEEK_END) != 0) {
        LOG_ERROR("Swap Manager: Error al posicionar en el final del archivo");
        fclose(swap_file);
        swap_file = NULL;
        free(swap_file_path);
        swap_file_path = NULL;
        list_destroy(process_blocks);
        process_blocks = NULL;
        return false;
    }
    
    long file_size = ftell(swap_file);
    if (file_size < 0) {
        LOG_ERROR("Swap Manager: Error al obtener tamaño del archivo");
        fclose(swap_file);
        swap_file = NULL;
        free(swap_file_path);
        swap_file_path = NULL;
        list_destroy(process_blocks);
        process_blocks = NULL;
        return false;
    }
    
    if (fseek(swap_file, 0, SEEK_SET) != 0) {
        LOG_ERROR("Swap Manager: Error al posicionar al inicio del archivo");
        fclose(swap_file);
        swap_file = NULL;
        free(swap_file_path);
        swap_file_path = NULL;
        list_destroy(process_blocks);
        process_blocks = NULL;
        return false;
    }
    
    total_pages = file_size / page_size;
    next_free_offset = file_size;
    
    // Inicializar bitmap
    if (total_pages > 0) {
        bitmap_size = total_pages;
        free_pages_bitmap = calloc(bitmap_size, sizeof(bool));
        if (free_pages_bitmap == NULL) {
            LOG_ERROR("Swap Manager: Error al crear bitmap de %u páginas", bitmap_size);
            fclose(swap_file);
            swap_file = NULL;
            free(swap_file_path);
            swap_file_path = NULL;
            list_destroy(process_blocks);
            process_blocks = NULL;
            return false;
        }
        // Marcar todas las páginas como libres inicialmente
        for (uint32_t i = 0; i < bitmap_size; i++) {
            free_pages_bitmap[i] = true;
        }
    } else {
        bitmap_size = 0;
        free_pages_bitmap = NULL;
    }

    LOG_INFO("Swap Manager: Inicializado con archivo de %ld bytes (%u paginas)", file_size, total_pages);
    return true;
}

void swap_manager_destroy() {
    if (swap_file != NULL) {
        fclose(swap_file);
        swap_file = NULL;
    }
    if (swap_file_path != NULL) {
        free(swap_file_path);
        swap_file_path = NULL;
    }
    if (process_blocks != NULL) {
        list_destroy_and_destroy_elements(process_blocks, free_swap_process_info);
        process_blocks = NULL;
    }
    if (free_pages_bitmap != NULL) {
        free(free_pages_bitmap);
        free_pages_bitmap = NULL;
    }
    bitmap_size = 0;
    total_pages = 0;
    next_free_offset = 0;
    page_size = 0;
    LOG_INFO("Swap Manager: Destruido y archivo SWAP cerrado");
}

t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages) {
    if (num_pages == 0) {
        return list_create();
    }

    if (swap_file == NULL) {
        LOG_ERROR("Swap Manager: Archivo de swap no inicializado");
        return NULL;
    }

    lock_swap_file();

    t_list* pages = list_create();
    if (pages == NULL) {
        LOG_ERROR("Swap Manager: Error al crear lista de paginas");
        unlock_swap_file();
        return NULL;
    }

    uint32_t start_offset;
    uint32_t start_page;
    
    // Intentar encontrar espacio libre usando bitmap
    start_offset = find_free_space_with_bitmap(num_pages);
    
    if (start_offset == (uint32_t)-1) {
        // No hay espacio libre, extender el archivo
        start_page = bitmap_size;
        start_offset = next_free_offset;
        
        // Extender bitmap
        if (!extend_bitmap_if_needed(num_pages)) {
            LOG_ERROR("Swap Manager: Error extendiendo bitmap");
            list_destroy(pages);
            unlock_swap_file();
            return NULL;
        }
        
        if (bitmap_size < start_page + num_pages) {
            LOG_ERROR("Swap Manager: Error en extensión del bitmap");
            list_destroy(pages);
            unlock_swap_file();
            return NULL;
        }
        
        next_free_offset += num_pages * page_size;
        total_pages = bitmap_size;
    } else {
        start_page = start_offset / page_size;
    }

    // Marcar páginas como ocupadas en el bitmap
    mark_pages_as_used(start_page, num_pages);

    // Crear información del bloque de proceso
    t_swap_process_info* process_block = malloc(sizeof(t_swap_process_info));
    if (process_block == NULL) {
        LOG_ERROR("Swap Manager: Error al asignar memoria para bloque de proceso");
        mark_pages_as_free(start_page, num_pages); // Rollback
        list_destroy(pages);
        unlock_swap_file();
        return NULL;
    }
    
    process_block->pid = pid;
    process_block->start_offset = start_offset;
    process_block->num_pages = num_pages;
    process_block->is_used = true;
    list_add(process_blocks, process_block);

    // Crear información de cada página
    for (uint32_t i = 0; i < num_pages; i++) {
        t_swap_page_info* page = malloc(sizeof(t_swap_page_info));
        if (page == NULL) {
            LOG_ERROR("Swap Manager: Error al asignar memoria para pagina %u", i);
            // Limpiar páginas ya creadas
            list_destroy_and_destroy_elements(pages, free);
            list_destroy(pages);
            mark_pages_as_free(start_page, num_pages); // Rollback
            unlock_swap_file();
            return NULL;
        }
        
        page->pid = pid;
        page->virtual_page_number = i;
        page->swap_offset = start_offset + (i * page_size);
        list_add(pages, page);
    }

    LOG_INFO("Swap Manager: Asignadas %u paginas en SWAP para PID %u en offset %u (pagina %u)", 
             num_pages, pid, start_offset, start_page);
    
    unlock_swap_file();
    return pages;
}

bool swap_write_pages(t_list* pages, void* process_memory, uint32_t page_size_param) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL || page_size_param == 0) {
        LOG_ERROR("Swap Manager: Parametros invalidos para escritura");
        return false;
    }

    lock_swap_file();

    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page = list_get(pages, i);
        if (page == NULL) {
            LOG_ERROR("Swap Manager: Página NULL en índice %d", i);
            unlock_swap_file();
            return false;
        }
        
        void* page_content = (char*)process_memory + (page->virtual_page_number * page_size_param);
        
        if (fseek(swap_file, page->swap_offset, SEEK_SET) != 0) {
            LOG_ERROR("Swap Manager: Error posicionando en offset %u", page->swap_offset);
            unlock_swap_file();
            return false;
        }
        
        size_t written = fwrite(page_content, 1, page_size_param, swap_file);
        if (written != page_size_param) {
            LOG_ERROR("Swap Manager: Error escribiendo pagina %u en offset %u (escrito %zu de %u bytes)", 
                     page->virtual_page_number, page->swap_offset, written, page_size_param);
            unlock_swap_file();
            return false;
        }
    }

    LOG_DEBUG("Swap Manager: Escritas %d paginas al archivo de swap", list_size(pages));
    unlock_swap_file();
    return true;
}

bool swap_read_pages(t_list* pages, void* process_memory, uint32_t page_size_param) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL || page_size_param == 0) {
        LOG_ERROR("Swap Manager: Parametros invalidos para lectura");
        return false;
    }

    lock_swap_file();

    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page = list_get(pages, i);
        if (page == NULL) {
            LOG_ERROR("Swap Manager: Página NULL en índice %d", i);
            unlock_swap_file();
            return false;
        }
        
        void* page_content = (char*)process_memory + (page->virtual_page_number * page_size_param);
        
        if (fseek(swap_file, page->swap_offset, SEEK_SET) != 0) {
            LOG_ERROR("Swap Manager: Error posicionando en offset %u", page->swap_offset);
            unlock_swap_file();
            return false;
        }
        
        size_t read = fread(page_content, 1, page_size_param, swap_file);
        if (read != page_size_param) {
            LOG_ERROR("Swap Manager: Error leyendo pagina %u desde offset %u (leido %zu de %u bytes)", 
                     page->virtual_page_number, page->swap_offset, read, page_size_param);
            unlock_swap_file();
            return false;
        }
    }

    LOG_DEBUG("Swap Manager: Leidas %d paginas del archivo de swap", list_size(pages));
    unlock_swap_file();
    return true;
}

void swap_free_pages(uint32_t pid) {
    if (process_blocks == NULL) {
        return;
    }

    lock_swap_file();

    // Buscar el bloque del proceso
    for (int i = 0; i < list_size(process_blocks); i++) {
        t_swap_process_info* block = list_get(process_blocks, i);
        if (block == NULL) {
            continue;
        }
        
        if (block->pid == pid && block->is_used) {
            // Marcar páginas como libres en el bitmap
            uint32_t start_page = block->start_offset / page_size;
            mark_pages_as_free(start_page, block->num_pages);
            
            // Marcar como libre (no eliminar, para poder reutilizar)
            block->is_used = false;
            LOG_INFO("Swap Manager: Proceso PID %u marcado como libre en offset %u (paginas %u-%u)", 
                     pid, block->start_offset, start_page, start_page + block->num_pages - 1);
            break;
        }
    }

    unlock_swap_file();
}

size_t swap_get_free_pages_count() {
    if (free_pages_bitmap == NULL) {
        return 0;
    }

    lock_swap_file();
    
    size_t free_pages = 0;
    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (free_pages_bitmap[i]) {
            free_pages++;
        }
    }
    
    unlock_swap_file();
    return free_pages;
}