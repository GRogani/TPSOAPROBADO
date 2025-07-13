#include "swap_manager.h"

extern INITIAL_SWAP_SIZE;

static FILE* swap_file = NULL;
static char* swap_file_path = NULL;

static uint32_t BLOCK_SIZE = 0;
static uint32_t MAX_BLOCKS = 0;

static char* blocks_bitmap = NULL;
static _Atomic uint32_t bitmap_size = 0;


static t_list* process_in_swap = NULL;


bool swap_manager_init(const t_memoria_config *config)
{
    if (!config || config->TAM_PAGINA == 0 || !config->PATH_SWAPFILE) {
        LOG_ERROR("Swap Manager: configuración inválida");
        return false;
    }

    BLOCK_SIZE = config->TAM_PAGINA;

    swap_file_path = strdup(config->PATH_SWAPFILE);
    if (!swap_file_path) {
        LOG_ERROR("Swap Manager: strdup(PATH_SWAPFILE) falló");
        return false;
    }

    
    swap_file = fopen(swap_file_path, "w+b");
    if (!swap_file) {
        LOG_ERROR("Swap Manager: no se pudo abrir %s", swap_file_path);
        free(swap_file_path);
        return false;
    }

    if (ftruncate(fileno(swap_file), INITIAL_SWAP_SIZE) != 0) {
        LOG_ERROR("Swap Manager: ftruncate(1MiB) falló: %s", strerror(errno));
        fclose(swap_file);
        free(swap_file_path);
        return false;
    }

    MAX_BLOCKS = INITIAL_SWAP_SIZE / BLOCK_SIZE;
    bitmap_size = MAX_BLOCKS;
    blocks_bitmap = (char*)safe_calloc(bitmap_size, sizeof(char));
    if (!blocks_bitmap) {
        LOG_ERROR("Swap Manager: calloc(bitmap) %u falló", bitmap_size);
        fclose(swap_file);
        free(swap_file_path);
        exit(1);
    }

    process_in_swap = list_create();
    if (!process_in_swap) {
        LOG_ERROR("Swap Manager: list_create() falló");
        free(blocks_bitmap);
        fclose(swap_file);
        free(swap_file_path);
        exit(1);
    }

    LOG_INFO("Swap  inicializado con %zuB (%u bloques de %uB)", (size_t)INITIAL_SWAP_SIZE, MAX_BLOCKS, BLOCK_SIZE);
    
}

void swap_manager_destroy() 
{
    if (swap_file != NULL) {
        fclose(swap_file);
        swap_file = NULL;
    }
    if (swap_file_path != NULL) {
        free(swap_file_path);
        swap_file_path = NULL;
    }
    if (process_in_swap != NULL) {
        list_destroy_and_destroy_elements(process_in_swap, free_swap_process_info);
        process_in_swap = NULL;
    }
    if (blocks_bitmap != NULL) {
        free(blocks_bitmap);
        blocks_bitmap = NULL;
    }
    bitmap_size = 0;
    MAX_BLOCKS = 0;
    BLOCK_SIZE = 0;
    
    LOG_INFO("Swap Manager: Destruido y archivo SWAP cerrado");
}


bool compare_by_offset(void* a, void* b) {
    if (a == NULL || b == NULL) return false;
    t_swap_process_info* block_a = (t_swap_process_info*)a;
    t_swap_process_info* block_b = (t_swap_process_info*)b;
    return block_a->start_offset < block_b->start_offset;
}


void free_swap_process_info(void* element) {
    if (element != NULL) {
        t_swap_process_info* info = (t_swap_process_info*)element;
        free(info);
    }
}


long find_free_space_with_bitmap(uint32_t blocks_needed) 
{
    if (blocks_bitmap == NULL) 
    {
        LOG_ERROR("FATAL: Bitmap no inicializado o bloques necesarios cero");
        exit(1);
    }

    uint32_t consecutive_free = 0;
    uint32_t start_index = 0;

    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (blocks_bitmap[i] == 0) {
            if (consecutive_free == 0) {
                start_index = i;
            }
            consecutive_free++;

            if (consecutive_free == blocks_needed) {
                return start_index;
            }
        } else {
            consecutive_free = 0;
        }
    }

    return -1;
}



void mark_pages_in_use(uint32_t start_page, uint32_t num_pages) 
{
    for (uint32_t i = 0; i < num_pages && (start_page + i) < bitmap_size; i++) {
        blocks_bitmap[start_page + i] = false;
    }
}


void mark_pages_as_free(uint32_t start_page, uint32_t num_pages) {
    if (blocks_bitmap == NULL || num_pages == 0) {
        return;
    }
    
    for (uint32_t i = 0; i < num_pages && (start_page + i) < bitmap_size; i++) {
        blocks_bitmap[start_page + i] = true;
    }
}

bool extend_bitmap(uint32_t blocks_needed) 
{
    uint32_t new_bitmap_size = bitmap_size + blocks_needed;

    char* new_bitmap = safe_realloc(blocks_bitmap, new_bitmap_size * sizeof(char));
    
    blocks_bitmap = new_bitmap;

    for (uint32_t i = bitmap_size; i < new_bitmap_size; i++) {
        blocks_bitmap[i] = true;
    }
    bitmap_size = new_bitmap_size;
    return true;
}

t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages) {
    if (num_pages == 0) {
        return NULL
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

        start_page = bitmap_size;

        if (!extend_bitmap(num_pages)) {
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
        
        next_free_offset += num_pages * BLOCK_SIZE;
        MAX_BLOCKS = bit;art_page = start_offset / BLOCK_SIZE;
    }
    

    // Marcar páginas como ocupadas en el bitmap
    mark_pages_in_use(start_page, num_pages);

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
    list_add(process_in_swap, process_block);

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
        page->swap_offset = start_offset + (i * BLOCK_SIZE);
        list_add(pages, page);
    }

    LOG_OBLIGATORIO("## SWAP - PID: %u - Asignadas %u páginas en offset %u (página %u)", pid, num_pages, start_offset, start_page);
    
    unlock_swap_file();
    return pages;
}

bool swap_write_pages(t_list* pages, void* process_memory, uint32_t BLOCK_SIZE_param) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL || BLOCK_SIZE_param == 0) {
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
        
        void* page_content = (char*)process_memory + (page->virtual_page_number * BLOCK_SIZE_param);
        
        if (fseek(swap_file, page->swap_offset, SEEK_SET) != 0) {
            LOG_ERROR("Swap Manager: Error posicionando en offset %u", page->swap_offset);
            unlock_swap_file();
            return false;
        }
        
        size_t written = fwrite(page_content, 1, BLOCK_SIZE_param, swap_file);
        if (written != BLOCK_SIZE_param) {
            LOG_ERROR("Swap Manager: Error escribiendo pagina %u en offset %u (escrito %zu de %u bytes)", 
                     page->virtual_page_number, page->swap_offset, written, BLOCK_SIZE_param);
            unlock_swap_file();
            return false;
        }
    }

    LOG_OBLIGATORIO("## SWAP - PID: %u - Escritura de %d páginas en swap", ((t_swap_page_info*)list_get(pages,0))->pid, list_size(pages));
    
    unlock_swap_file();
    return true;
}

bool swap_read_pages(t_list* pages, void* process_memory, uint32_t BLOCK_SIZE_param) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL || BLOCK_SIZE_param == 0) {
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
        
        void* page_content = (char*)process_memory + (page->virtual_page_number * BLOCK_SIZE_param);
        
        if (fseek(swap_file, page->swap_offset, SEEK_SET) != 0) {
            LOG_ERROR("Swap Manager: Error posicionando en offset %u", page->swap_offset);
            unlock_swap_file();
            return false;
        }
        
        size_t read = fread(page_content, 1, BLOCK_SIZE_param, swap_file);
        if (read != BLOCK_SIZE_param) {
            LOG_ERROR("Swap Manager: Error leyendo pagina %u desde offset %u (leido %zu de %u bytes)", 
                     page->virtual_page_number, page->swap_offset, read, BLOCK_SIZE_param);
            unlock_swap_file();
            return false;
        }
    }

    LOG_OBLIGATORIO("## SWAP - PID: %u - Lectura de %d páginas desde swap", ((t_swap_page_info*)list_get(pages,0))->pid, list_size(pages));
    
    unlock_swap_file();
    return true;
}

void swap_free_pages(uint32_t pid) {
    if (process_in_swap == NULL) {
        return;
    }

    lock_swap_file();

    // Buscar el bloque del proceso
    for (int i = 0; i < list_size(process_in_swap); i++) {
        t_swap_process_info* block = list_get(process_in_swap, i);
        if (block == NULL) {
            continue;
        }
        
        if (block->pid == pid && block->is_used) {
            // Marcar páginas como libres en el bitmap
            uint32_t start_page = block->start_offset / BLOCK_SIZE;
            mark_pages_as_free(start_page, block->num_pages);
            
            // Marcar como libre (no eliminar, para poder reutilizar)
            block->is_used = false;
            LOG_OBLIGATORIO("## SWAP - PID: %u - Liberadas %u páginas en offset %u (páginas %u-%u)", pid, block->num_pages, block->start_offset, start_page, start_page + block->num_pages - 1);
            break;
        }
    }

    unlock_swap_file();
}

size_t swap_get_free_pages_count() {
    if (blocks_bitmap == NULL) {
        return 0;
    }

    lock_swap_file();
    
    size_t free_pages = 0;
    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (blocks_bitmap[i]) {
            free_pages++;
        }
    }
    
    unlock_swap_file();
    return free_pages;
}