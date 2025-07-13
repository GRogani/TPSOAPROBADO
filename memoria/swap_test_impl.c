#define INITIAL_SWAP_SIZE (1024 * 1024)

// Static variables for the module
static FILE* swap_file = NULL;                // Swap file
static char* swap_file_path = NULL;           // Path to swap file
static uint32_t page_size = 0;                // Configured page size
static uint32_t swap_delay_ms = 0;            // SWAP delay in ms

// Additional mutex and RWLock for synchronization
static pthread_mutex_t blocks_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t file_rwlock = PTHREAD_RWLOCK_INITIALIZER;

// List of blocks (used and free)
static t_list* swap_blocks = NULL;

// Usage statistics
static _Atomic uint32_t total_pages = 0;
static _Atomic uint32_t used_pages = 0;
static _Atomic uint32_t read_ops = 0;
static _Atomic uint32_t write_ops = 0;
static _Atomic uint32_t allocated_processes = 0;

// Static function prototypes
static bool compare_by_offset(void* a, void* b);
static t_swap_block* find_best_free_block(uint32_t num_pages);
static bool extend_swap_file(uint32_t additional_pages);
static void apply_swap_delay(void);

/**
 * @brief Compare two swap blocks by their offset for sorting
 */
static bool compare_by_offset(void* a, void* b) {
    t_swap_block* block_a = (t_swap_block*)a;
    t_swap_block* block_b = (t_swap_block*)b;
    return block_a->start_offset < block_b->start_offset;
}

/**
 * @brief Apply the configured swap delay
 */
static void apply_swap_delay(void) {
    if (swap_delay_ms > 0) {
        LOG_DEBUG("Aplicando retardo SWAP de %u ms", swap_delay_ms);
        usleep(swap_delay_ms * 1000);
    }
}

/**
 * @brief Find the best free block that can fit the given number of pages
 * Uses a best-fit strategy for allocation
 */
static t_swap_block* find_best_free_block(uint32_t num_pages) {
    if (list_size(swap_blocks) == 0) {
        return NULL;
    }
    
    pthread_mutex_lock(&blocks_mutex);
    
    t_swap_block* best_block = NULL;
    uint32_t best_size = UINT32_MAX;
    uint32_t required_size = num_pages * page_size;
    
    // Find the smallest free block that fits the requirement
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        
        if (!block->is_used && block->pid == 0) {
            uint32_t block_size = block->num_pages * page_size;
            
            if (block_size >= required_size && block_size < best_size) {
                best_block = block;
                best_size = block_size;
                
                // If we found a perfect fit, use it immediately
                if (block_size == required_size) {
                    break;
                }
            }
        }
    }
    
    // If we found a suitable block that's larger than needed, split it
    if (best_block && best_block->num_pages > num_pages) {
        // Create a new block for the remaining space
        t_swap_block* new_block = malloc(sizeof(t_swap_block));
        if (new_block) {
            new_block->pid = 0;  // Mark as free
            new_block->is_used = false;
            new_block->start_offset = best_block->start_offset + (num_pages * page_size);
            new_block->num_pages = best_block->num_pages - num_pages;
            
            // Adjust the size of the best block
            best_block->num_pages = num_pages;
            
            // Add the new block to our list and resort by offset
            list_add(swap_blocks, new_block);
        }
    }
    
    pthread_mutex_unlock(&blocks_mutex);
    return best_block;
}

/**
 * @brief Extend the swap file to accommodate more pages
 */
static bool extend_swap_file(uint32_t additional_pages) {
    if (swap_file == NULL) {
        return false;
    }
    
    LOG_INFO("Extendiendo archivo SWAP para %u páginas adicionales (%u bytes)", 
           additional_pages, additional_pages * page_size);
    
    // Lock the file for writing
    pthread_rwlock_wrlock(&file_rwlock);
    
    // Get current file size
    fseek(swap_file, 0, SEEK_END);
    long current_size = ftell(swap_file);
    
    // Extend the file with zeros
    uint32_t extension_size = additional_pages * page_size;
    char* zeros = calloc(1, extension_size);
    if (!zeros) {
        pthread_rwlock_unlock(&file_rwlock);
        LOG_ERROR("No se pudo asignar memoria para extender el archivo SWAP");
        return false;
    }
    
    fwrite(zeros, 1, extension_size, swap_file);
    fflush(swap_file);
    free(zeros);
    
    // Create a new free block for the extended area
    pthread_mutex_lock(&blocks_mutex);
    
    t_swap_block* block = malloc(sizeof(t_swap_block));
    if (!block) {
        pthread_rwlock_unlock(&file_rwlock);
        pthread_mutex_unlock(&blocks_mutex);
        LOG_ERROR("No se pudo asignar memoria para el bloque de extensión");
        return false;
    }
    
    block->pid = 0;  // PID 0 means free block
    block->is_used = false;
    block->start_offset = current_size;
    block->num_pages = additional_pages;
    clock_gettime(CLOCK_MONOTONIC, &block->last_access);
    
    list_add(swap_blocks, block);
    
    // Update total pages count
    total_pages += additional_pages;
    
    pthread_mutex_unlock(&blocks_mutex);
    pthread_rwlock_unlock(&file_rwlock);
    
    LOG_INFO("Archivo SWAP extendido exitosamente. Tamaño actual: %ld bytes", current_size + extension_size);
    
    return true;
}

/**
 * @brief Initialize the SWAP management system
 */
bool swap_manager_init(const t_memoria_config* config) {
    LOG_INFO("Inicializando sistema de SWAP mejorado...");

    // Save configuration
    page_size = config->TAM_PAGINA;
    swap_delay_ms = config->RETARDO_SWAP;
    swap_file_path = strdup(config->PATH_SWAPFILE);

    // Create blocks list
    swap_blocks = list_create();
    if (swap_blocks == NULL) {
        LOG_ERROR("Error al crear la lista de bloques de SWAP");
        return false;
    }

    // Check if file already exists
    bool exists = (access(swap_file_path, F_OK) == 0);

    // Open or create the SWAP file
    swap_file = fopen(swap_file_path, "r+b");
    if (swap_file == NULL) {
        // If it doesn't exist, create it
        swap_file = fopen(swap_file_path, "w+b");
        if (swap_file == NULL) {
            LOG_ERROR("Error al crear el archivo de SWAP en '%s'", swap_file_path);
            list_destroy(swap_blocks);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }
    }

    // If it's a new file, initialize it
    if (!exists) {
        LOG_INFO("Creando archivo SWAP inicial de %d bytes", INITIAL_SWAP_SIZE);
        
        // Create initial empty block
        t_swap_block* initial_block = malloc(sizeof(t_swap_block));
        if (initial_block == NULL) {
            LOG_ERROR("Error al asignar memoria para el bloque inicial");
            fclose(swap_file);
            swap_file = NULL;
            list_destroy(swap_blocks);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }

        // Initialize empty block
        initial_block->pid = 0;  // PID 0 indicates free block
        initial_block->is_used = false;
        initial_block->start_offset = 0;
        initial_block->num_pages = INITIAL_SWAP_SIZE / page_size;
        clock_gettime(CLOCK_MONOTONIC, &initial_block->last_access);

        // Add to blocks list
        list_add(swap_blocks, initial_block);

        // Update total pages
        total_pages = initial_block->num_pages;

        // Fill with zeros
        char* zeros = calloc(1, INITIAL_SWAP_SIZE);
        if (zeros == NULL) {
            LOG_ERROR("No se pudo asignar memoria para inicializar el archivo SWAP");
            fclose(swap_file);
            swap_file = NULL;
            free(initial_block);
            list_destroy(swap_blocks);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }

        // Write zeros to initialize file
        size_t written = fwrite(zeros, 1, INITIAL_SWAP_SIZE, swap_file);
        fflush(swap_file);
        free(zeros);

        if (written != INITIAL_SWAP_SIZE) {
            LOG_ERROR("Error al inicializar el archivo SWAP. Escritos %zu de %d bytes", 
                    written, INITIAL_SWAP_SIZE);
            fclose(swap_file);
            swap_file = NULL;
            free(initial_block);
            list_destroy(swap_blocks);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }
    } else {
        LOG_INFO("Abriendo archivo SWAP existente: %s", swap_file_path);
        
        // Get file size
        fseek(swap_file, 0, SEEK_END);
        long file_size = ftell(swap_file);
        fseek(swap_file, 0, SEEK_SET);
        
        // Create initial free block
        t_swap_block* initial_block = malloc(sizeof(t_swap_block));
        if (initial_block == NULL) {
            LOG_ERROR("Error al asignar memoria para el bloque inicial");
            fclose(swap_file);
            swap_file = NULL;
            list_destroy(swap_blocks);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }
        
        // Initialize block
        initial_block->pid = 0;
        initial_block->is_used = false;
        initial_block->start_offset = 0;
        initial_block->num_pages = file_size / page_size;
        clock_gettime(CLOCK_MONOTONIC, &initial_block->last_access);
        
        // Add to blocks list
        list_add(swap_blocks, initial_block);
        
        // Update total pages
        total_pages = initial_block->num_pages;
    }

    LOG_INFO("Sistema de SWAP inicializado correctamente. Total de páginas: %u", total_pages);
    return true;
}

/**
 * @brief Clean up the SWAP management system
 */
void swap_manager_destroy(void) {
    if (swap_file != NULL) {
        fclose(swap_file);
        swap_file = NULL;
    }
    
    if (swap_blocks != NULL) {
        list_destroy_and_destroy_elements(swap_blocks, free);
        swap_blocks = NULL;
    }
    
    if (swap_file_path != NULL) {
        free(swap_file_path);
        swap_file_path = NULL;
    }
    
    LOG_INFO("Sistema de SWAP finalizado correctamente");
}

/**
 * @brief Allocate pages in the swap file for a complete process
 */
t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages) {
    if (swap_file == NULL || page_size == 0) {
        LOG_ERROR("Sistema de SWAP no inicializado");
        return NULL;
    }
    
    LOG_INFO("Asignando %u páginas para proceso %u", num_pages, pid);
    
    // Find a free block big enough
    t_swap_block* free_block = find_best_free_block(num_pages);
    
    // If there's no suitable block, try to extend the file
    if (free_block == NULL) {
        LOG_INFO("No hay espacio suficiente en SWAP. Intentando extender...");
        
        // Try to extend with additional 50% to avoid frequent extensions
        uint32_t additional_pages = num_pages + (num_pages / 2);
        if (!extend_swap_file(additional_pages)) {
            LOG_ERROR("No se pudo extender el archivo SWAP");
            return NULL;
        }
        
        // Try again
        free_block = find_best_free_block(num_pages);
        if (free_block == NULL) {
            LOG_ERROR("No se pudo encontrar espacio suficiente en SWAP aun después de extender");
            return NULL;
        }
    }
    
    // Mark the block as used
    pthread_mutex_lock(&blocks_mutex);
    free_block->pid = pid;
    free_block->is_used = true;
    clock_gettime(CLOCK_MONOTONIC, &free_block->last_access);
    pthread_mutex_unlock(&blocks_mutex);
    
    // Create a list with the assigned pages information
    t_list* pages_info = list_create();
    if (pages_info == NULL) {
        LOG_ERROR("Error al crear la lista de información de páginas");
        return NULL;
    }
    
    // Initialize each page
    for (uint32_t i = 0; i < num_pages; i++) {
        t_swap_page_info* page_info = malloc(sizeof(t_swap_page_info));
        if (page_info == NULL) {
            LOG_ERROR("Error al asignar memoria para información de página");
            list_destroy_and_destroy_elements(pages_info, free);
            return NULL;
        }
        
        page_info->pid = pid;
        page_info->virtual_page_number = i;
        page_info->swap_offset = free_block->start_offset + (i * page_size);
        
        list_add(pages_info, page_info);
    }
    
    // Update statistics
    used_pages += num_pages;
    allocated_processes++;
    
    // Log
    LOG_OBLIGATORIO("SWAP: Asignadas %u páginas para proceso %u en offset %u", 
                   num_pages, pid, free_block->start_offset);
    
    return pages_info;
}

/**
 * @brief Read all pages of a process from the swap file
 */
bool swap_read_pages(t_list* pages, void* process_memory, uint32_t page_size) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL) {
        LOG_ERROR("Parámetros inválidos para lectura de páginas en SWAP");
        return false;
    }
    
    // Iterate through all pages
    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page_info = list_get(pages, i);
        void* page_buffer = process_memory + (page_info->virtual_page_number * page_size);
        
        // Apply configured delay
        apply_swap_delay();
        
        // Acquire read lock for the file
        pthread_rwlock_rdlock(&file_rwlock);
        
        // Position at the correct offset
        fseek(swap_file, page_info->swap_offset, SEEK_SET);
        
        // Read the page
        size_t read_result = fread(page_buffer, 1, page_size, swap_file);
        
        // Release the lock
        pthread_rwlock_unlock(&file_rwlock);
        
        // Check result
        if (read_result != page_size) {
            LOG_ERROR("Error al leer página de SWAP. PID: %u, VPN: %u, Offset: %u, Leídos: %zu de %u",
                    page_info->pid, page_info->virtual_page_number, page_info->swap_offset, read_result, page_size);
            return false;
        }
        
        // Update statistics
        read_ops++;
        
        // Update timestamp of the corresponding block
        pthread_mutex_lock(&blocks_mutex);
        for (int j = 0; j < list_size(swap_blocks); j++) {
            t_swap_block* block = list_get(swap_blocks, j);
            if (block->pid == page_info->pid && 
                page_info->swap_offset >= block->start_offset && 
                page_info->swap_offset < block->start_offset + (block->num_pages * page_size)) {
                
                clock_gettime(CLOCK_MONOTONIC, &block->last_access);
                break;
            }
        }
        pthread_mutex_unlock(&blocks_mutex);
        
        LOG_INFO("SWAP: Leída página %u del proceso %u desde offset %u", 
                page_info->virtual_page_number, page_info->pid, page_info->swap_offset);
    }
    
    return true;
}

/**
 * @brief Write all pages of a process to the swap file
 */
bool swap_write_pages(t_list* pages, void* process_memory, uint32_t page_size) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL) {
        LOG_ERROR("Parámetros inválidos para escritura de páginas en SWAP");
        return false;
    }
    
    // Iterate through all pages
    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page_info = list_get(pages, i);
        void* page_buffer = process_memory + (page_info->virtual_page_number * page_size);
        
        // Apply configured delay
        apply_swap_delay();
        
        // Acquire write lock for the file
        pthread_rwlock_wrlock(&file_rwlock);
        
        // Position at the correct offset
        fseek(swap_file, page_info->swap_offset, SEEK_SET);
        
        // Write the page
        size_t write_result = fwrite(page_buffer, 1, page_size, swap_file);
        fflush(swap_file);
        
        // Release the lock
        pthread_rwlock_unlock(&file_rwlock);
        
        // Check result
        if (write_result != page_size) {
            LOG_ERROR("Error al escribir página en SWAP. PID: %u, VPN: %u, Offset: %u, Escritos: %zu de %u",
                    page_info->pid, page_info->virtual_page_number, page_info->swap_offset, write_result, page_size);
            return false;
        }
        
        // Update statistics
        write_ops++;
        
        // Update timestamp of the corresponding block
        pthread_mutex_lock(&blocks_mutex);
        for (int j = 0; j < list_size(swap_blocks); j++) {
            t_swap_block* block = list_get(swap_blocks, j);
            if (block->pid == page_info->pid && 
                page_info->swap_offset >= block->start_offset && 
                page_info->swap_offset < block->start_offset + (block->num_pages * page_size)) {
                
                clock_gettime(CLOCK_MONOTONIC, &block->last_access);
                break;
            }
        }
        pthread_mutex_unlock(&blocks_mutex);
        
        LOG_INFO("SWAP: Escrita página %u del proceso %u en offset %u", 
                page_info->virtual_page_number, page_info->pid, page_info->swap_offset);
    }
    
    return true;
}

/**
 * @brief Free the pages of a process in the swap file (mark as free)
 */
void swap_free_pages(uint32_t pid) {
    if (swap_file == NULL || swap_blocks == NULL) {
        LOG_ERROR("Sistema de SWAP no inicializado");
        return;
    }
    
    LOG_INFO("Liberando páginas del proceso %u en SWAP", pid);
    
    pthread_mutex_lock(&blocks_mutex);
    
    // Find all blocks belonging to this process
    int freed_blocks = 0;
    uint32_t freed_pages = 0;
    
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        
        if (block->pid == pid) {
            // Mark as free
            block->pid = 0;
            block->is_used = false;
            freed_blocks++;
            freed_pages += block->num_pages;
            
            LOG_INFO("SWAP: Liberado bloque en offset %u con %u páginas", 
                   block->start_offset, block->num_pages);
        }
    }
    
    // Update statistics
    if (freed_pages > 0) {
        used_pages -= freed_pages;
        allocated_processes--;
        
        LOG_OBLIGATORIO("SWAP: Liberadas %u páginas del proceso %u", freed_pages, pid);
    } else {
        LOG_INFO("SWAP: No se encontraron páginas del proceso %u", pid);
    }
    
    // Merge adjacent free blocks to reduce fragmentation
    bool merged;
    do {
        merged = false;
        
        for (int i = 0; i < list_size(swap_blocks) - 1; i++) {
            t_swap_block* current = list_get(swap_blocks, i);
            t_swap_block* next = list_get(swap_blocks, i + 1);
            
            // If both blocks are free and adjacent
            if (current->pid == 0 && next->pid == 0 && 
                current->start_offset + (current->num_pages * page_size) == next->start_offset) {
                
                // Merge the blocks
                current->num_pages += next->num_pages;
                
                // Remove the second block
                free(next);
                
                // Remove from list
                list_remove(swap_blocks, i + 1);
                
                merged = true;
                break;
            }
        }
    } while (merged);
    
    pthread_mutex_unlock(&blocks_mutex);
}

/**
 * @brief Get the current number of free pages in the swap file
 */
size_t swap_get_free_pages_count(void) {
    if (swap_blocks == NULL) {
        return 0;
    }
    
    pthread_mutex_lock(&blocks_mutex);
    
    size_t free_pages = 0;
    
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == 0 && !block->is_used) {
            free_pages += block->num_pages;
        }
    }
    
    pthread_mutex_unlock(&blocks_mutex);
    
    return free_pages;
}

// Additional function to match our implementation
void list_remove(t_list* list, int index) {
    if(index < 0 || index >= list->size) return;
    
    // Move all elements after the removed one
    for(int i = index; i < list->size - 1; i++) {
        list->elements[i] = list->elements[i + 1];
    }
    
    list->size--;
    list->elements = realloc(list->elements, sizeof(void*) * list->size);
}
