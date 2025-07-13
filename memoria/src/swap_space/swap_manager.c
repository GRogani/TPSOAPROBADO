#include "swap_manager.h"
#include "swap_structures.h"
#include "../../../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <errno.h>
#include <ctype.h>

// Definir CLOCK_REALTIME si no está definido
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

// Definir pthread_rwlock_t si es necesario
#ifndef PTHREAD_RWLOCK_INITIALIZER
typedef pthread_mutex_t pthread_rwlock_t;
#define PTHREAD_RWLOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define pthread_rwlock_init(rwlock, attr) pthread_mutex_init(rwlock, attr)
#define pthread_rwlock_destroy(rwlock) pthread_mutex_destroy(rwlock)
#define pthread_rwlock_rdlock(rwlock) pthread_mutex_lock(rwlock)
#define pthread_rwlock_wrlock(rwlock) pthread_mutex_lock(rwlock)
#define pthread_rwlock_unlock(rwlock) pthread_mutex_unlock(rwlock)
#endif

// Tamaño inicial del archivo de swap (128MB como ejemplo)
// Variables estáticas del módulo
static FILE* swap_file = NULL;                // Archivo de swap
static char* swap_file_path = NULL;           // Ruta al archivo de swap
static uint32_t page_size = 0;                // Tamaño de página configurado
static uint32_t swap_delay_ms = 0;            // Retardo de SWAP en ms

// Mutex y RWLock para sincronización
static pthread_mutex_t blocks_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t file_rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Lista de bloques (usados y libres)
static t_list* swap_blocks = NULL;

// Estadísticas de uso
static atomic_uint_least32_t total_pages = 0;
static atomic_uint_least32_t used_pages = 0;
static atomic_uint_least32_t read_ops = 0;
static atomic_uint_least32_t write_ops = 0;
static atomic_uint_least32_t allocated_processes = 0;

// Estructura para representar el estado del swap
static t_swap_status swap_status = {0};

// Prototipos de funciones estáticas (internas)
static bool compare_by_offset(void* a, void* b);
static t_swap_block* find_best_free_block(uint32_t num_pages);
static bool extend_swap_file(uint32_t additional_pages);
static void apply_swap_delay(void);
static bool swap_read_page(t_swap_page_info* page_info, void* buffer);
static bool swap_write_page(t_swap_page_info* page_info, void* buffer);
static void merge_adjacent_free_blocks(void);
static void update_block_timestamp(t_swap_block* block);
static bool try_compact_swap_file(void);

// Prototipos de funciones estáticas (internas)
static bool compare_by_offset(void* a, void* b);
static t_swap_block* find_best_free_block(uint32_t num_pages);
static bool extend_swap_file(uint32_t additional_pages);
static void apply_swap_delay(void);
static bool swap_read_page(t_swap_page_info* page_info, void* buffer);
static bool swap_write_page(t_swap_page_info* page_info, void* buffer);
static void merge_adjacent_free_blocks(void);
static void update_block_timestamp(t_swap_block* block);
static bool try_compact_swap_file(void);

/**
 * @brief Compara dos bloques de swap por su offset para ordenación
 * 
 * @param a Primer bloque a comparar
 * @param b Segundo bloque a comparar
 * @return true si el primer bloque tiene un offset menor
 */
static bool compare_by_offset(void* a, void* b) {
    t_swap_block* block_a = (t_swap_block*)a;
    t_swap_block* block_b = (t_swap_block*)b;
    return block_a->start_offset < block_b->start_offset;
}

/**
 * @brief Aplica el retardo configurado para las operaciones de swap
 */
static void apply_swap_delay(void) {
    if (swap_delay_ms > 0) {
        struct timespec ts;
        ts.tv_sec = swap_delay_ms / 1000;
        ts.tv_nsec = (swap_delay_ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
}

/**
 * @brief Inicializa el sistema de gestión de SWAP mejorado
 * 
 * @param config Configuración de memoria con parámetros de SWAP
 * @return bool - true si la inicialización fue exitosa, false en caso de error
 */
bool swap_manager_init(const t_memoria_config* config) {
    LOG_INFO("Inicializando sistema de SWAP mejorado...");

    // Guardar configuración
    page_size = config->TAM_PAGINA;
    swap_delay_ms = config->RETARDO_SWAP;
    swap_file_path = strdup(config->PATH_SWAPFILE);

    // Crear lista de bloques
    swap_blocks = list_create();
    if (swap_blocks == NULL) {
        LOG_ERROR("Error al crear la lista de bloques de SWAP");
        return false;
    }

    // Verificar si el archivo ya existe
    bool exists = (access(swap_file_path, F_OK) == 0);

    // Abrir o crear el archivo de SWAP
    swap_file = fopen(swap_file_path, "r+b");
    if (swap_file == NULL) {
        // Si no existe, crearlo
        swap_file = fopen(swap_file_path, "w+b");
        if (swap_file == NULL) {
            LOG_ERROR("Error al crear el archivo de SWAP en '%s': %s", 
                     swap_file_path, strerror(errno));
            list_destroy(swap_blocks);
            free(swap_file_path);
            swap_file_path = NULL;
            return false;
        }
    }

    // Si es un archivo nuevo, inicializarlo
    if (!exists) {
        LOG_INFO("Creando archivo SWAP inicial de %d bytes", INITIAL_SWAP_SIZE);
        
        // Crear un bloque inicial vacío
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

        // Inicializar el bloque vacío
        initial_block->pid = 0;  // PID 0 indica bloque libre
        initial_block->start_offset = 0;
        initial_block->num_pages = INITIAL_SWAP_SIZE / page_size;
        initial_block->is_used = 0;  // Marcar como libre
        clock_gettime(CLOCK_REALTIME, &initial_block->last_access);
        
        // Agregar el bloque a la lista
        list_add(swap_blocks, initial_block);

        // Extender el archivo al tamaño inicial
        fseek(swap_file, INITIAL_SWAP_SIZE - 1, SEEK_SET);
        fputc('\0', swap_file);
        fflush(swap_file);
        
        // Actualizar estadísticas
        total_pages = initial_block->num_pages;
    } else {
        // Para archivos existentes, se debería reconstruir la estructura de bloques
        // En esta implementación, inicializamos como un único bloque libre
        struct stat file_stat;
        stat(swap_file_path, &file_stat);
        uint32_t file_size = file_stat.st_size;
        
        // Crear un bloque libre con todo el espacio disponible
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
        
        initial_block->pid = 0;  // Bloque libre
        initial_block->start_offset = 0;
        initial_block->num_pages = file_size / page_size;
        initial_block->is_used = 0;
        clock_gettime(CLOCK_REALTIME, &initial_block->last_access);
        
        list_add(swap_blocks, initial_block);
        
        // Actualizar estadísticas
        atomic_store(&total_pages, initial_block->num_pages);
    }

    // Inicializar la estructura de estado para métricas
    atomic_store(&swap_status.total_pages, atomic_load(&total_pages));
    atomic_store(&swap_status.used_pages, 0);
    atomic_store(&swap_status.read_operations, 0);
    atomic_store(&swap_status.write_operations, 0);
    atomic_store(&swap_status.allocated_processes, 0);

    LOG_INFO("Sistema de SWAP inicializado exitosamente. Archivo: %s, Total páginas: %u, Tamaño página: %u", 
             swap_file_path, atomic_load(&total_pages), page_size);
    return true;
}

/**
 * @brief Libera todos los recursos utilizados por el gestor de SWAP
 */
void swap_manager_destroy(void) {
    LOG_INFO("Liberando sistema de SWAP...");
    
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
    
    // Destruir mutex y rwlock
    pthread_mutex_destroy(&blocks_mutex);
    pthread_rwlock_destroy(&file_rwlock);
    
    LOG_INFO("Sistema de SWAP liberado correctamente");
}

/**
 * @brief Comparador para ordenar bloques por offset
 */
/**
 * @brief Encuentra el mejor bloque libre para asignar (best-fit)
 * 
 * @param num_pages Número de páginas necesarias
 * @return t_swap_block* - Mejor bloque encontrado o NULL si no hay espacio suficiente
 */
static t_swap_block* find_best_free_block(uint32_t num_pages) {
    if (swap_blocks == NULL || list_is_empty(swap_blocks)) {
        return NULL;
    }
    
    // Necesitamos lock para buscar
    pthread_mutex_lock(&blocks_mutex);
    
    t_swap_block* best_block = NULL;
    uint32_t best_fit_size = UINT32_MAX;
    
    // Buscar el mejor bloque que se ajuste
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block && block->is_used == 0 && block->num_pages >= num_pages) {
            if (block->num_pages < best_fit_size) {
                best_fit_size = block->num_pages;
                best_block = block;
            }
        }
    }
    
    // Si encontramos un bloque adecuado
    if (best_block != NULL) {
        // Si el bloque es más grande que lo necesario, dividirlo
        if (best_block->num_pages > num_pages) {
            t_swap_block* remainder_block = malloc(sizeof(t_swap_block));
            if (remainder_block != NULL) {
                // Inicializar el bloque restante
                remainder_block->pid = 0;
                remainder_block->start_offset = best_block->start_offset + (num_pages * page_size);
                remainder_block->num_pages = best_block->num_pages - num_pages;
                remainder_block->is_used = 0;
                clock_gettime(CLOCK_REALTIME, &remainder_block->last_access);
                
                // Añadir el bloque restante a la lista
                list_add(swap_blocks, remainder_block);
                
                // Ajustar el tamaño del bloque encontrado
                best_block->num_pages = num_pages;
            }
        }
    }
    
    // Ordenar la lista por offset para mantener el orden
    list_sort(swap_blocks, compare_by_offset);
    
    pthread_mutex_unlock(&blocks_mutex);
    return best_block;
}

/**
 * @brief Amplia el archivo de swap si no hay suficiente espacio
 * 
 * @param additional_pages Número de páginas adicionales a agregar
 * @return bool - true si la operación fue exitosa
 */
static bool extend_swap_file(uint32_t additional_pages) {
    LOG_INFO("Ampliando archivo SWAP en %u páginas (%u bytes)",
             additional_pages, additional_pages * page_size);
             
    // Obtener el tamaño actual del archivo
    struct stat file_stat;
    stat(swap_file_path, &file_stat);
    uint32_t current_size = file_stat.st_size;
    
    // Crear un nuevo bloque para representar el espacio adicional
    t_swap_block* new_block = malloc(sizeof(t_swap_block));
    if (new_block == NULL) {
        LOG_ERROR("Error al asignar memoria para el nuevo bloque de SWAP");
        return false;
    }
    
    // Inicializar el bloque nuevo
    new_block->pid = 0;
    new_block->start_offset = current_size;
    new_block->num_pages = additional_pages;
    new_block->is_used = 0;
    clock_gettime(CLOCK_REALTIME, &new_block->last_access);
    
    // Bloquear acceso
    pthread_rwlock_wrlock(&file_rwlock);
    pthread_mutex_lock(&blocks_mutex);
    
    // Extender el archivo
    fseek(swap_file, current_size + (additional_pages * page_size) - 1, SEEK_SET);
    int result = fputc('\0', swap_file);
    fflush(swap_file);
    
    if (result == EOF) {
        LOG_ERROR("Error al extender el archivo SWAP");
        free(new_block);
        pthread_mutex_unlock(&blocks_mutex);
        pthread_rwlock_unlock(&file_rwlock);
        return false;
    }
    
    // Agregar el nuevo bloque a la lista
    list_add(swap_blocks, new_block);
    
    // Ordenar la lista por offset
    list_sort(swap_blocks, compare_by_offset);
    
    // Combinar bloques adyacentes libres si existen
    bool combined = true;
    while (combined) {
        combined = false;
        for (int i = 0; i < list_size(swap_blocks) - 1; i++) {
            t_swap_block* current = list_get(swap_blocks, i);
            t_swap_block* next = list_get(swap_blocks, i + 1);
            
            if (current && next && 
                current->is_used == 0 && next->is_used == 0 &&
                current->start_offset + (current->num_pages * page_size) == next->start_offset) {
                
                // Combinar los bloques
                current->num_pages += next->num_pages;
                
                // Eliminar el segundo bloque
                list_remove_and_destroy_element(swap_blocks, i + 1, free);
                
                combined = true;
                break;
            }
        }
    }
    
    // Actualizar estadísticas
    atomic_fetch_add(&total_pages, additional_pages);
    atomic_fetch_add(&swap_status.total_pages, additional_pages);
    
    pthread_mutex_unlock(&blocks_mutex);
    pthread_rwlock_unlock(&file_rwlock);
    
    LOG_INFO("Archivo SWAP ampliado exitosamente. Nuevo tamaño: %u páginas", atomic_load(&total_pages));
    return true;
}

// Function defined above

/**
 * @brief Asigna páginas en el archivo de SWAP para un proceso
 * 
 * @param pid ID del proceso
 * @param num_pages Número de páginas a asignar
 * @return t_list* Lista de t_swap_page_info o NULL si hubo error
 */
t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages) {
    if (swap_file == NULL || page_size == 0) {
        LOG_ERROR("Sistema de SWAP no inicializado");
        return NULL;
    }
    
    LOG_OBLIGATORIO("SWAP_ALLOCATE: Asignando %u páginas para proceso %u", num_pages, pid);
    
    // Primero verificar si ya hay páginas asignadas para este proceso
    // Si es así, debemos liberarlas para evitar duplicación
    pthread_mutex_lock(&blocks_mutex);
    bool has_existing_pages = false;
    
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == pid && block->is_used == 1) {
            has_existing_pages = true;
            LOG_OBLIGATORIO("SWAP_ALLOCATE: PID %u ya tiene páginas asignadas en swap, liberando primero", pid);
            break;
        }
    }
    pthread_mutex_unlock(&blocks_mutex);
    
    // Si ya había páginas para este proceso, liberarlas primero
    if (has_existing_pages) {
        swap_free_pages(pid);
        LOG_OBLIGATORIO("SWAP_ALLOCATE: Páginas anteriores del proceso %u liberadas", pid);
    }
    
    // Encontrar un bloque libre suficientemente grande
    t_swap_block* free_block = find_best_free_block(num_pages);
    
    // Si no hay un bloque adecuado, intentar compactar primero
    if (free_block == NULL) {
        LOG_INFO("No se encontró bloque libre adecuado. Intentando compactar el archivo SWAP...");
        
        if (try_compact_swap_file()) {
            // Intentar de nuevo después de compactar
            free_block = find_best_free_block(num_pages);
        }
    }
    
    // Si aún no hay un bloque adecuado, intentar extender el archivo
    if (free_block == NULL) {
        LOG_INFO("No hay espacio suficiente en SWAP después de compactar. Intentando extender...");
        
        // Intentar extender con un 50% adicional de lo que se necesita para evitar
        // extensiones frecuentes
        uint32_t additional_pages = num_pages + (num_pages / 2);
        if (!extend_swap_file(additional_pages)) {
            LOG_ERROR("No se pudo extender el archivo SWAP");
            return NULL;
        }
        
        // Intentar de nuevo
        free_block = find_best_free_block(num_pages);
        if (free_block == NULL) {
            LOG_ERROR("No se pudo encontrar espacio suficiente en SWAP aun después de extender");
            return NULL;
        }
    }
    
    // Marcar el bloque como usado
    pthread_mutex_lock(&blocks_mutex);
    free_block->pid = pid;
    free_block->is_used = 1;  // Marcar como usado
    clock_gettime(CLOCK_REALTIME, &free_block->last_access);
    pthread_mutex_unlock(&blocks_mutex);
    
    // Crear una lista con la información de las páginas asignadas
    t_list* pages_info = list_create();
    if (pages_info == NULL) {
        LOG_ERROR("Error al crear la lista de información de páginas");
        return NULL;
    }
    
    LOG_OBLIGATORIO("SWAP_ALLOCATE: Bloque asignado para PID %u, offset %u, %u páginas", 
              pid, free_block->start_offset, num_pages);
    
    // Inicializar cada página y asignarla correctamente
    for (uint32_t i = 0; i < num_pages; i++) {
        t_swap_page_info* page_info = malloc(sizeof(t_swap_page_info));
        if (page_info == NULL) {
            LOG_ERROR("Error al asignar memoria para información de página");
            list_destroy_and_destroy_elements(pages_info, free);
            return NULL;
        }
        
        page_info->pid = pid;
        page_info->virtual_page_number = i;  // Número de página virtual
        page_info->swap_offset = free_block->start_offset + (i * page_size);  // Posición en el archivo de swap
        
        LOG_OBLIGATORIO("SWAP_ALLOCATE: PID %u, VPN %u -> Swap offset: %u", 
                  pid, i, page_info->swap_offset);
        
        list_add(pages_info, page_info);
    }
    
    // Borrar contenido previo en el archivo swap para este bloque
    // Esto evita que los datos antiguos persistan
    pthread_rwlock_wrlock(&file_rwlock);
    char* zero_buffer = calloc(1, page_size);
    if (zero_buffer != NULL) {
        for (uint32_t i = 0; i < num_pages; i++) {
            uint32_t offset = free_block->start_offset + (i * page_size);
            fseek(swap_file, offset, SEEK_SET);
            fwrite(zero_buffer, 1, page_size, swap_file);
        }
        fflush(swap_file);
        free(zero_buffer);
        LOG_OBLIGATORIO("SWAP_ALLOCATE: Se ha limpiado el área de swap para el proceso %u", pid);
    }
    pthread_rwlock_unlock(&file_rwlock);
    
    // Actualizar estadísticas
    atomic_fetch_add(&used_pages, num_pages);
    atomic_fetch_add(&swap_status.used_pages, num_pages);
    atomic_fetch_add(&allocated_processes, 1);
    atomic_fetch_add(&swap_status.allocated_processes, 1);
    
    return pages_info;
}

/**
 * @brief Lee todas las páginas de un proceso desde el archivo de swap
 *
 * @param pages Lista con información de páginas del proceso
 * @param process_memory Puntero a la memoria donde cargar los datos leídos
 * @param page_size Tamaño de cada página
 * @return true si éxito, false si error
 */
bool swap_read_pages(t_list* pages, void* process_memory, uint32_t page_size) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL) {
        LOG_ERROR("Parámetros inválidos para lectura de páginas en SWAP");
        return false;
    }
    
    LOG_OBLIGATORIO("SWAP_READ: Iniciando lectura de %d páginas de proceso desde swap", list_size(pages));
    
    // Verificamos que todas las páginas pertenecen al mismo proceso
    uint32_t pid = 0;
    if (list_size(pages) > 0) {
        t_swap_page_info* first_page = list_get(pages, 0);
        pid = first_page->pid;
    }
    
    // Crear un mapa para asegurarnos que cada página virtual se lee exactamente una vez
    // y se coloca en la posición correcta de memoria
    bool* page_read = calloc(list_size(pages), sizeof(bool));
    if (!page_read) {
        LOG_ERROR("Error al asignar memoria para control de lectura de páginas");
        return false;
    }
    
    // Limpiar la memoria del proceso antes de cargar las páginas
    memset(process_memory, 0, page_size * list_size(pages));
    
    // Iterar por todas las páginas
    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page_info = list_get(pages, i);
        
        // Verificar que este proceso sea el dueño de la página
        if (page_info->pid != pid) {
            LOG_ERROR("SWAP_READ: Error - página con PID %u encontrada en lista de proceso %u",
                     page_info->pid, pid);
            free(page_read);
            return false;
        }
        
        // Calcular la dirección virtual y el offset en memoria del proceso
        uint32_t virtual_page_num = page_info->virtual_page_number;
        
        // Verificar que esta página virtual no la hayamos leído antes
        if (virtual_page_num < list_size(pages) && page_read[virtual_page_num]) {
            LOG_WARNING("SWAP_READ: Página virtual %u ya fue leída anteriormente, saltando duplicado", 
                       virtual_page_num);
            continue;
        }
        
        uint32_t virt_addr_offset = virtual_page_num * page_size;
        void* page_buffer = process_memory + virt_addr_offset;
        
        LOG_OBLIGATORIO("SWAP_READ: PID %u, Página virtual %u (offset %u) <- offset swap %u",
                 page_info->pid, virtual_page_num, virt_addr_offset, page_info->swap_offset);
        
        // Aplicar retardo configurado
        apply_swap_delay();
        
        // Adquirir lock de lectura para el archivo
        pthread_rwlock_rdlock(&file_rwlock);
        
        // Posicionarse en el offset correcto
        fseek(swap_file, page_info->swap_offset, SEEK_SET);
        
        // Leer la página
        size_t read_result = fread(page_buffer, 1, page_size, swap_file);
        
        // Liberar el lock
        pthread_rwlock_unlock(&file_rwlock);
        
        // Verificar resultado
        if (read_result != page_size) {
            LOG_ERROR("Error al leer página de SWAP. PID: %u, VPN: %u, Offset: %u, Leídos: %zu de %u",
                    page_info->pid, page_info->virtual_page_number, page_info->swap_offset, read_result, page_size);
            free(page_read);
            return false;
        }
        
        // Marcar la página como leída
        if (virtual_page_num < list_size(pages)) {
            page_read[virtual_page_num] = true;
        }
        
        // Mostrar los primeros bytes para depuración
        char debug_data[17] = {0};
        memcpy(debug_data, page_buffer, 16 < page_size ? 16 : page_size);
        for (int j = 0; j < 16 && j < page_size; j++) {
            if (!isprint(debug_data[j])) debug_data[j] = '.';
        }
        LOG_OBLIGATORIO("SWAP_READ: Datos leídos (primeros bytes): '%s'", debug_data);
        
        // Actualizar estadísticas
        atomic_fetch_add(&read_ops, 1);
        atomic_fetch_add(&swap_status.read_operations, 1);
        
        // Actualizar timestamp del bloque correspondiente
        pthread_mutex_lock(&blocks_mutex);
        for (int j = 0; j < list_size(swap_blocks); j++) {
            t_swap_block* block = list_get(swap_blocks, j);
            if (block->pid == page_info->pid && 
                page_info->swap_offset >= block->start_offset && 
                page_info->swap_offset < block->start_offset + (block->num_pages * page_size)) {
                
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                block->last_access = ts;
                break;
            }
        }
        pthread_mutex_unlock(&blocks_mutex);
        
        LOG_OBLIGATORIO("SWAP: Leída página %u del proceso %u desde offset %u", 
                page_info->virtual_page_number, page_info->pid, page_info->swap_offset);
    }
    
    free(page_read);
    return true;
    
    return true;
}

/**
 * @brief Lee datos de una página en el archivo de SWAP (función interna)
 * 
 * @param page_info Información de la página a leer
 * @param buffer Buffer donde se almacenarán los datos leídos
 * @return bool - true si la lectura fue exitosa
 */
static bool swap_read_page(t_swap_page_info* page_info, void* buffer) {
    if (swap_file == NULL || page_info == NULL || buffer == NULL) {
        LOG_ERROR("Parámetros inválidos para lectura de página en SWAP");
        return false;
    }
    
    // Aplicar retardo configurado
    apply_swap_delay();
    
    // Adquirir lock de lectura para el archivo
    pthread_rwlock_rdlock(&file_rwlock);
    
    // Posicionarse en el offset correcto
    fseek(swap_file, page_info->swap_offset, SEEK_SET);
    
    // Leer la página
    size_t read_result = fread(buffer, 1, page_size, swap_file);
    
    // Liberar el lock
    pthread_rwlock_unlock(&file_rwlock);
    
    // Verificar resultado
    if (read_result != page_size) {
        LOG_ERROR("Error al leer página de SWAP. PID: %u, VPN: %u, Offset: %u, Leídos: %zu de %u",
                 page_info->pid, page_info->virtual_page_number, page_info->swap_offset, read_result, page_size);
        return false;
    }
    
    // Actualizar estadísticas
    read_ops++;
    
    // Actualizar timestamp del bloque correspondiente
    pthread_mutex_lock(&blocks_mutex);
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == page_info->pid && 
            page_info->swap_offset >= block->start_offset && 
            page_info->swap_offset < block->start_offset + (block->num_pages * page_size)) {
            
            clock_gettime(CLOCK_REALTIME, &block->last_access);
            break;
        }
    }
    pthread_mutex_unlock(&blocks_mutex);
    
    LOG_INFO("SWAP: Leída página %u del proceso %u desde offset %u", 
             page_info->virtual_page_number, page_info->pid, page_info->swap_offset);
    
    return true;
}

/**
 * @brief Escribe todas las páginas de un proceso al archivo de swap
 *
 * @param pages Lista con información de páginas del proceso
 * @param process_memory Puntero a la memoria del proceso
 * @param page_size Tamaño de cada página
 * @return true si éxito, false si error
 */
bool swap_write_pages(t_list* pages, void* process_memory, uint32_t page_size) {
    if (swap_file == NULL || pages == NULL || process_memory == NULL) {
        LOG_ERROR("Parámetros inválidos para escritura de páginas en SWAP");
        return false;
    }
    
    LOG_OBLIGATORIO("SWAP_WRITE: Iniciando escritura de %d páginas de proceso en swap", list_size(pages));
    
    // Verificamos que todas las páginas pertenecen al mismo proceso
    uint32_t pid = 0;
    if (list_size(pages) > 0) {
        t_swap_page_info* first_page = list_get(pages, 0);
        pid = first_page->pid;
    }
    
    // Crear un mapa para asegurarnos que cada página virtual se escribe exactamente una vez
    // y en la ubicación correcta de swap
    bool* page_written = calloc(list_size(pages), sizeof(bool));
    if (!page_written) {
        LOG_ERROR("Error al asignar memoria para control de escritura de páginas");
        return false;
    }
    
    // Iterar por todas las páginas
    for (int i = 0; i < list_size(pages); i++) {
        t_swap_page_info* page_info = list_get(pages, i);
        
        // Verificar que este proceso sea el dueño de la página
        if (page_info->pid != pid) {
            LOG_ERROR("SWAP_WRITE: Error - página con PID %u encontrada en lista de proceso %u",
                     page_info->pid, pid);
            free(page_written);
            return false;
        }
        
        // Calcular la dirección virtual y el offset en memoria del proceso
        uint32_t virtual_page_num = page_info->virtual_page_number;
        
        // Verificar que esta página virtual no la hayamos escrito antes
        if (virtual_page_num < list_size(pages) && page_written[virtual_page_num]) {
            LOG_WARNING("SWAP_WRITE: Página virtual %u ya fue escrita anteriormente, saltando duplicado", 
                       virtual_page_num);
            continue;
        }
        
        uint32_t virt_addr_offset = virtual_page_num * page_size;
        void* page_buffer = process_memory + virt_addr_offset;
        
        LOG_OBLIGATORIO("SWAP_WRITE: PID %u, Página virtual %u (offset %u) -> offset swap %u",
                 page_info->pid, virtual_page_num, virt_addr_offset, page_info->swap_offset);
        
        // Mostrar los primeros bytes para depuración
        char debug_data[17] = {0};
        memcpy(debug_data, page_buffer, 16 < page_size ? 16 : page_size);
        for (int j = 0; j < 16 && j < page_size; j++) {
            if (!isprint(debug_data[j])) debug_data[j] = '.';
        }
        LOG_OBLIGATORIO("SWAP_WRITE: Datos a escribir (primeros bytes): '%s'", debug_data);
        
        // Aplicar retardo configurado
        apply_swap_delay();
        
        // Adquirir lock de escritura para el archivo
        pthread_rwlock_wrlock(&file_rwlock);
        
        // Posicionarse en el offset correcto
        fseek(swap_file, page_info->swap_offset, SEEK_SET);
        
        // Escribir la página
        size_t write_result = fwrite(page_buffer, 1, page_size, swap_file);
        fflush(swap_file);
        
        // Verificar la escritura leyendo el contenido de nuevo
        char* verify_buffer = malloc(page_size);
        if (verify_buffer) {
            fseek(swap_file, page_info->swap_offset, SEEK_SET);
            fread(verify_buffer, 1, page_size, swap_file);
            
            // Verificar contenido
            if (memcmp(page_buffer, verify_buffer, page_size) != 0) {
                LOG_WARNING("SWAP_WRITE: Verificación fallida - contenido leído no coincide con lo escrito");
            } else {
                LOG_DEBUG("SWAP_WRITE: Verificación exitosa - contenido correctamente escrito");
            }
            
            free(verify_buffer);
        }
        
        // Liberar el lock
        pthread_rwlock_unlock(&file_rwlock);
        
        // Verificar resultado
        if (write_result != page_size) {
            LOG_ERROR("Error al escribir página en SWAP. PID: %u, VPN: %u, Offset: %u, Escritos: %zu de %u",
                    page_info->pid, page_info->virtual_page_number, page_info->swap_offset, write_result, page_size);
            free(page_written);
            return false;
        }
        
        // Marcar la página como escrita
        if (virtual_page_num < list_size(pages)) {
            page_written[virtual_page_num] = true;
        }
        
        // Actualizar estadísticas
        atomic_fetch_add(&write_ops, 1);
        atomic_fetch_add(&swap_status.write_operations, 1);
        
        // Actualizar timestamp del bloque correspondiente
        pthread_mutex_lock(&blocks_mutex);
        for (int j = 0; j < list_size(swap_blocks); j++) {
            t_swap_block* block = list_get(swap_blocks, j);
            if (block->pid == page_info->pid && 
                page_info->swap_offset >= block->start_offset && 
                page_info->swap_offset < block->start_offset + (block->num_pages * page_size)) {
                
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                block->last_access = ts;
                break;
            }
        }
        pthread_mutex_unlock(&blocks_mutex);
        
        LOG_OBLIGATORIO("SWAP: Escrita página %u del proceso %u en offset %u", 
                page_info->virtual_page_number, page_info->pid, page_info->swap_offset);
    }
    
    free(page_written);
    return true;
}

/**
 * @brief Escribe datos en una página en el archivo de SWAP (función interna)
 * 
 * @param page_info Información de la página a escribir
 * @param buffer Buffer con los datos a escribir
 * @return bool - true si la escritura fue exitosa
 */
static bool swap_write_page(t_swap_page_info* page_info, void* buffer) {
    if (swap_file == NULL || page_info == NULL || buffer == NULL) {
        LOG_ERROR("Parámetros inválidos para escritura de página en SWAP");
        return false;
    }
    
    // Aplicar retardo configurado
    apply_swap_delay();
    
    // Adquirir lock de escritura para el archivo
    pthread_rwlock_wrlock(&file_rwlock);
    
    // Posicionarse en el offset correcto
    fseek(swap_file, page_info->swap_offset, SEEK_SET);
    
    // Escribir la página
    size_t write_result = fwrite(buffer, 1, page_size, swap_file);
    fflush(swap_file);
    
    // Liberar el lock
    pthread_rwlock_unlock(&file_rwlock);
    
    // Verificar resultado
    if (write_result != page_size) {
        LOG_ERROR("Error al escribir página en SWAP. PID: %u, VPN: %u, Offset: %u, Escritos: %zu de %u",
                 page_info->pid, page_info->virtual_page_number, page_info->swap_offset, write_result, page_size);
        return false;
    }
    
    // Actualizar estadísticas
    write_ops++;
    
    // Actualizar timestamp del bloque correspondiente
    pthread_mutex_lock(&blocks_mutex);
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block->pid == page_info->pid && 
            page_info->swap_offset >= block->start_offset && 
            page_info->swap_offset < block->start_offset + (block->num_pages * page_size)) {
            
            clock_gettime(CLOCK_REALTIME, &block->last_access);
            break;
        }
    }
    pthread_mutex_unlock(&blocks_mutex);
    
    LOG_INFO("SWAP: Escrita página %u del proceso %u en offset %u", 
             page_info->virtual_page_number, page_info->pid, page_info->swap_offset);
    
    return true;
}

/**
 * @brief Libera todas las páginas asignadas a un proceso en SWAP
 * 
 * @param pid ID del proceso
 */
void swap_free_pages(uint32_t pid) {
    if (swap_file == NULL) {
        LOG_ERROR("Parámetros inválidos para liberar páginas en SWAP: swap_file es NULL");
        return;
    }
    
    LOG_INFO("Liberando páginas del proceso %u en SWAP", pid);
    
    uint32_t total_freed = 0;
    bool found = false;
    
    // Adquirir lock para la lista de bloques
    pthread_mutex_lock(&blocks_mutex);
    
    // Buscar y liberar todos los bloques del proceso
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block == NULL) {
            continue;
        }
        
        if (block->pid == pid && block->is_used) {
            // Marcar como libre
            block->is_used = 0;
            block->pid = 0;  // PID 0 indica bloque libre
            
            found = true;
            total_freed += block->num_pages;
        }
    }
    
    // Si se liberaron bloques, combinar bloques adyacentes
    if (found) {
        // Primero liberamos el mutex para llamar a merge_adjacent_free_blocks
        // que adquirirá el mismo mutex
        pthread_mutex_unlock(&blocks_mutex);
        
        // Combinar bloques adyacentes libres para reducir fragmentación
        merge_adjacent_free_blocks();  // Usamos nuestra función auxiliar que adquirirá su propio mutex
        
        // Actualizar estadísticas atómicamente sin necesidad del mutex
        atomic_fetch_sub(&used_pages, total_freed);
        atomic_fetch_sub(&swap_status.used_pages, total_freed);
        atomic_fetch_sub(&allocated_processes, 1);
        atomic_fetch_sub(&swap_status.allocated_processes, 1);
        
        LOG_OBLIGATORIO("SWAP: Liberadas %u páginas del proceso %u", total_freed, pid);
    } else {
        LOG_WARNING("No se encontraron páginas asignadas al proceso %u en SWAP", pid);
        pthread_mutex_unlock(&blocks_mutex);
    }
    
    // No hacemos unlock aquí porque ya lo hicimos en ambos caminos de ejecución
}

/**
 * @brief Obtiene el estado actual del sistema de SWAP
 * 
 * @return t_swap_status Estado actual del sistema de SWAP
 */
t_swap_status swap_get_status(void) {
    t_swap_status status;
    
    status.total_pages = atomic_load(&total_pages);
    status.used_pages = atomic_load(&used_pages);
    status.read_operations = atomic_load(&read_ops);
    status.write_operations = atomic_load(&write_ops);
    status.allocated_processes = atomic_load(&allocated_processes);
    
    return status;
}

/**
 * @brief Obtiene la cantidad actual de páginas libres en el archivo de swap.
 * @return La cantidad de páginas libres.
 */
size_t swap_get_free_pages_count(void) {
    size_t free_pages = 0;
    
    pthread_mutex_lock(&blocks_mutex);
    
    for (int i = 0; i < list_size(swap_blocks); i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        if (block && block->is_used == 0) {
            free_pages += block->num_pages;
        }
    }
    
    pthread_mutex_unlock(&blocks_mutex);
    
    return free_pages;
}

/**
 * @brief Compacta el archivo de swap moviendo los bloques usados al principio
 * para reducir la fragmentación
 *
 * @return true si se realizó la compactación exitosamente
 */
static bool try_compact_swap_file(void) {
    if (swap_file == NULL) {
        LOG_ERROR("Sistema de SWAP no inicializado");
        return false;
    }
    
    LOG_INFO("Iniciando compactación del archivo SWAP...");
    
    // Necesitamos lock exclusivo para toda la operación
    pthread_rwlock_wrlock(&file_rwlock);
    pthread_mutex_lock(&blocks_mutex);
    
    // Ordenar los bloques por offset
    list_sort(swap_blocks, compare_by_offset);
    
    // Verificar si es necesaria la compactación
    bool needs_compaction = false;
    uint32_t num_free_blocks = 0;
    
    for (int i = 0; i < list_size(swap_blocks) - 1; i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        t_swap_block* next = list_get(swap_blocks, i + 1);
        
        if (block->is_used == 0 && next->is_used == 1) {
            needs_compaction = true;
            break;
        }
        
        if (block->is_used == 0) {
            num_free_blocks++;
        }
    }
    
    // Si el último bloque está libre, incrementar contador
    t_swap_block* last = list_get(swap_blocks, list_size(swap_blocks) - 1);
    if (last && last->is_used == 0) {
        num_free_blocks++;
    }
    
    // Si no hay bloques libres o no es necesaria la compactación, salir
    if (num_free_blocks == 0 || !needs_compaction) {
        pthread_mutex_unlock(&blocks_mutex);
        pthread_rwlock_unlock(&file_rwlock);
        LOG_INFO("No es necesaria la compactación del archivo SWAP");
        return false;
    }
    
    // Crear un buffer temporal para mover los datos
    char* temp_buffer = malloc(page_size);
    if (temp_buffer == NULL) {
        pthread_mutex_unlock(&blocks_mutex);
        pthread_rwlock_unlock(&file_rwlock);
        LOG_ERROR("Error al asignar memoria para la compactación de SWAP");
        return false;
    }
    
    // Realizar la compactación
    uint32_t target_offset = 0;
    bool success = true;
    
    for (int i = 0; i < list_size(swap_blocks) && success; i++) {
        t_swap_block* block = list_get(swap_blocks, i);
        
        if (block->is_used == 1) {  // Bloque usado
            if (block->start_offset != target_offset) {
                // Mover los datos del bloque
                for (uint32_t page = 0; page < block->num_pages; page++) {
                    uint32_t src_offset = block->start_offset + (page * page_size);
                    uint32_t dst_offset = target_offset + (page * page_size);
                    
                    // Leer página
                    fseek(swap_file, src_offset, SEEK_SET);
                    size_t read_result = fread(temp_buffer, 1, page_size, swap_file);
                    if (read_result != page_size) {
                        LOG_ERROR("Error al leer página durante la compactación: offset=%u", src_offset);
                        success = false;
                        break;
                    }
                    
                    // Escribir página en nueva ubicación
                    fseek(swap_file, dst_offset, SEEK_SET);
                    size_t write_result = fwrite(temp_buffer, 1, page_size, swap_file);
                    if (write_result != page_size) {
                        LOG_ERROR("Error al escribir página durante la compactación: offset=%u", dst_offset);
                        success = false;
                        break;
                    }
                    
                    // Actualizar las estructuras de páginas para todos los procesos
                    for (int j = 0; j < list_size(swap_blocks); j++) {
                        t_swap_block* proc_block = list_get(swap_blocks, j);
                        if (proc_block->pid == block->pid) {
                            // Actualizar offsets
                            if (proc_block->start_offset == src_offset) {
                                proc_block->start_offset = dst_offset;
                            }
                        }
                    }
                }
                
                // Actualizar el offset del bloque
                block->start_offset = target_offset;
            }
            
            target_offset += block->num_pages * page_size;
        }
    }
    
    // Liberar el buffer temporal
    free(temp_buffer);
    
    if (success) {
        // Crear un único bloque libre al final
        uint32_t free_pages = 0;
        
        // Eliminar todos los bloques libres
        for (int i = list_size(swap_blocks) - 1; i >= 0; i--) {
            t_swap_block* block = list_get(swap_blocks, i);
            if (block->is_used == 0) {
                free_pages += block->num_pages;
                list_remove_and_destroy_element(swap_blocks, i, free);
            }
        }
        
        if (free_pages > 0) {
            // Crear un nuevo bloque libre al final
            t_swap_block* free_block = malloc(sizeof(t_swap_block));
            if (free_block != NULL) {
                free_block->pid = 0;
                free_block->is_used = 0;
                free_block->start_offset = target_offset;
                free_block->num_pages = free_pages;
                clock_gettime(CLOCK_REALTIME, &free_block->last_access);
                list_add(swap_blocks, free_block);
            }
        }
        
        LOG_INFO("Compactación del archivo SWAP completada exitosamente");
    }
    
    pthread_mutex_unlock(&blocks_mutex);
    pthread_rwlock_unlock(&file_rwlock);
    
    return success;
}

/**
 * @brief Actualiza el timestamp de un bloque para indicar acceso reciente
 * @param block Bloque a actualizar
 */
static void update_block_timestamp(t_swap_block* block) {
    if (block != NULL) {
        clock_gettime(CLOCK_REALTIME, &block->last_access);
    }
}

/**
 * @brief Combina bloques adyacentes libres para reducir fragmentación
 */
static void merge_adjacent_free_blocks(void) {
    bool merged;
    
    pthread_mutex_lock(&blocks_mutex);
    
    // Ordenar los bloques por offset
    list_sort(swap_blocks, compare_by_offset);
    
    do {
        merged = false;
        for (int i = 0; i < list_size(swap_blocks) - 1; i++) {
            t_swap_block* current = list_get(swap_blocks, i);
            t_swap_block* next = list_get(swap_blocks, i + 1);
            
            if (current && next && 
                current->is_used == 0 && next->is_used == 0 &&
                current->start_offset + (current->num_pages * page_size) == next->start_offset) {
                
                // Combinar los bloques
                current->num_pages += next->num_pages;
                
                // Eliminar el segundo bloque
                list_remove_and_destroy_element(swap_blocks, i + 1, free);
                
                merged = true;
                break;
            }
        }
    } while (merged);
    
    pthread_mutex_unlock(&blocks_mutex);
}

/**
 * @brief Libera memoria asociada a una estructura de información de proceso en swap
 * @param element Puntero a la estructura a liberar
 */
void free_swap_process_info(void* element) {
    if (element) {
        t_swap_page_info* info = (t_swap_page_info*)element;
        free(info);
    }
}
