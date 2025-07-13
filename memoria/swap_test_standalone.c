#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

// Definiciones básicas
#define PATH_SWAPFILE "/home/utnso/test_swapfile.bin"
#define SWAP_PAGE_SIZE 64
#define INITIAL_SWAP_SIZE (64 * 16)  // 16 páginas iniciales

// Estructuras para el manejo de swap
typedef struct {
    uint32_t offset;   // Desplazamiento en el archivo de swap
    uint32_t pid;      // ID del proceso al que pertenece
    uint32_t index;    // Índice de página dentro del proceso
} t_swap_page_info;

// Definición forward del struct para evitar problemas de referencia circular
typedef struct swap_block t_swap_block;

// Definición completa de la estructura del bloque de swap
struct swap_block {
    uint32_t offset;       // Desplazamiento en bytes desde el inicio del archivo
    uint32_t num_pages;    // Número de páginas en este bloque
    uint32_t pid;          // ID del proceso al que pertenece (0 si está libre)
    t_swap_block* next;    // Puntero al siguiente bloque
    t_swap_block* prev;    // Puntero al bloque anterior
};

typedef struct {
    char* path;                // Ruta del archivo de swap
    int fd;                    // Descriptor de archivo
    uint32_t total_pages;      // Número total de páginas en el archivo
    uint32_t free_pages;       // Número de páginas libres
    uint32_t page_size;        // Tamaño de página en bytes
    t_swap_block* first_block; // Primer bloque en la lista
    pthread_mutex_t mutex;     // Mutex para operaciones de asignación
    pthread_rwlock_t rwlock;   // RWLock para operaciones de lectura/escritura
} t_swap_status;

t_log* logger;
t_swap_status* swap_status = NULL;

// Inicializa el logger
void init_logger() {
    logger = log_create("swap_test.log", "SWAP_TEST", true, LOG_LEVEL_INFO);
}

// Crea un nuevo bloque de swap
t_swap_block* create_block(uint32_t offset, uint32_t num_pages, uint32_t pid) {
    t_swap_block* block = malloc(sizeof(t_swap_block));
    if (block == NULL) {
        log_error(logger, "Error de memoria al crear bloque de swap");
        return NULL;
    }
    
    block->offset = offset;
    block->num_pages = num_pages;
    block->pid = pid;
    block->next = NULL;
    block->prev = NULL;
    
    return block;
}

// Inserta un bloque en la lista enlazada
void insert_block(t_swap_block* block) {
    if (swap_status->first_block == NULL) {
        swap_status->first_block = block;
        return;
    }
    
    // Buscamos la posición correcta basada en offset
    t_swap_block* current = swap_status->first_block;
    t_swap_block* prev = NULL;
    
    while (current != NULL && current->offset < block->offset) {
        prev = current;
        current = current->next;
    }
    
    // Insertamos el bloque
    if (prev == NULL) {
        // Insertar al principio
        block->next = swap_status->first_block;
        swap_status->first_block->prev = block;
        swap_status->first_block = block;
    } else {
        // Insertar en medio o al final
        block->next = current;
        block->prev = prev;
        prev->next = block;
        
        if (current != NULL) {
            current->prev = block;
        }
    }
}

// Une bloques libres adyacentes
bool merge_adjacent_free_blocks(t_swap_block* block) {
    bool merged = false;
    
    // Intentar fusionar con el bloque siguiente
    if (block->next != NULL && block->next->pid == 0) {
        t_swap_block* next_block = block->next;
        
        block->num_pages += next_block->num_pages;
        block->next = next_block->next;
        
        if (next_block->next != NULL) {
            next_block->next->prev = block;
        }
        
        free(next_block);
        merged = true;
    }
    
    // Intentar fusionar con el bloque anterior
    if (block->prev != NULL && block->prev->pid == 0) {
        t_swap_block* prev_block = block->prev;
        
        prev_block->num_pages += block->num_pages;
        prev_block->next = block->next;
        
        if (block->next != NULL) {
            block->next->prev = prev_block;
        }
        
        free(block);
        merged = true;
    }
    
    return merged;
}

// Inicializa el sistema de swap
bool init_swap() {
    // Inicializamos la estructura de estado
    swap_status = malloc(sizeof(t_swap_status));
    if (swap_status == NULL) {
        log_error(logger, "Error al asignar memoria para swap_status");
        return false;
    }
    
    swap_status->path = strdup(PATH_SWAPFILE);
    swap_status->page_size = SWAP_PAGE_SIZE;
    swap_status->first_block = NULL;
    pthread_mutex_init(&swap_status->mutex, NULL);
    pthread_rwlock_init(&swap_status->rwlock, NULL);
    
    // Creamos o truncamos el archivo de swap
    swap_status->fd = open(swap_status->path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (swap_status->fd == -1) {
        log_error(logger, "Error al abrir archivo de swap: %s", swap_status->path);
        free(swap_status->path);
        free(swap_status);
        return false;
    }
    
    // Ajustamos el tamaño del archivo
    uint32_t file_size = INITIAL_SWAP_SIZE;
    if (ftruncate(swap_status->fd, file_size) == -1) {
        log_error(logger, "Error al ajustar el tamaño del archivo de swap");
        close(swap_status->fd);
        free(swap_status->path);
        free(swap_status);
        return false;
    }
    
    // Inicializamos los valores
    swap_status->total_pages = INITIAL_SWAP_SIZE / SWAP_PAGE_SIZE;
    swap_status->free_pages = swap_status->total_pages;
    
    // Creamos el primer bloque (libre)
    t_swap_block* initial_block = create_block(0, swap_status->total_pages, 0);
    swap_status->first_block = initial_block;
    
    log_info(logger, "Sistema de swap inicializado. Tamaño: %u bytes, Páginas: %u, Tamaño de página: %u bytes", 
             file_size, swap_status->total_pages, SWAP_PAGE_SIZE);
    
    return true;
}

// Extiende el archivo de swap cuando se necesita más espacio
bool extend_swap_file(uint32_t pages_needed) {
    log_info(logger, "Extendiendo archivo de swap. Páginas necesarias: %u", pages_needed);
    
    pthread_rwlock_wrlock(&swap_status->rwlock);
    
    // Calculamos el nuevo tamaño (doble del actual o lo necesario, el mayor)
    uint32_t current_size = swap_status->total_pages * swap_status->page_size;
    uint32_t pages_to_add = (swap_status->total_pages > pages_needed) ? 
                            swap_status->total_pages : pages_needed;
    uint32_t new_size = current_size + (pages_to_add * swap_status->page_size);
    
    // Extendemos el archivo
    if (ftruncate(swap_status->fd, new_size) == -1) {
        log_error(logger, "Error al extender el archivo de swap");
        pthread_rwlock_unlock(&swap_status->rwlock);
        return false;
    }
    
    // Actualizamos los contadores
    uint32_t old_total = swap_status->total_pages;
    swap_status->total_pages += pages_to_add;
    swap_status->free_pages += pages_to_add;
    
    // Creamos un nuevo bloque libre al final
    uint32_t new_block_offset = old_total * swap_status->page_size;
    t_swap_block* new_block = create_block(new_block_offset, pages_to_add, 0);
    
    // Añadimos el bloque a la lista
    t_swap_block* current = swap_status->first_block;
    if (current == NULL) {
        swap_status->first_block = new_block;
    } else {
        // Vamos al último bloque
        while (current->next != NULL) {
            current = current->next;
        }
        
        current->next = new_block;
        new_block->prev = current;
        
        // Si el bloque anterior también está libre, fusionamos
        if (current->pid == 0) {
            merge_adjacent_free_blocks(current);
        }
    }
    
    log_info(logger, "Archivo de swap extendido. Nuevo tamaño: %u bytes, Páginas totales: %u", 
             new_size, swap_status->total_pages);
    
    pthread_rwlock_unlock(&swap_status->rwlock);
    return true;
}

// Compacta el archivo de swap para reducir la fragmentación
bool try_compact_swap() {
    log_info(logger, "Intentando compactar el archivo de swap...");
    
    pthread_mutex_lock(&swap_status->mutex);
    
    bool changes_made = false;
    t_swap_block* current = swap_status->first_block;
    
    while (current != NULL) {
        // Si el bloque está libre, intentamos fusionarlo con adyacentes
        if (current->pid == 0 && current->next != NULL) {
            if (merge_adjacent_free_blocks(current)) {
                changes_made = true;
                // Como la estructura puede haber cambiado, volvemos al principio
                // pero continuamos desde el bloque actual
                continue;
            }
        }
        current = current->next;
    }
    
    if (changes_made) {
        log_info(logger, "Compactación de swap completada. Bloques fusionados.");
    } else {
        log_info(logger, "Compactación de swap completada. No se encontraron bloques para fusionar.");
    }
    
    pthread_mutex_unlock(&swap_status->mutex);
    return changes_made;
}

// Busca espacio para asignar páginas
t_swap_page_info* allocate_swap_pages(uint32_t pid, uint32_t num_pages) {
    log_info(logger, "Solicitando asignación de %u páginas para proceso %u", num_pages, pid);
    
    if (num_pages == 0) {
        log_error(logger, "Error: Intentando asignar 0 páginas");
        return NULL;
    }
    
    pthread_mutex_lock(&swap_status->mutex);
    
    // Verificamos si hay suficientes páginas libres
    if (swap_status->free_pages < num_pages) {
        log_error(logger, "No hay suficientes páginas libres. Solicitadas: %u, Disponibles: %u", 
                 num_pages, swap_status->free_pages);
        pthread_mutex_unlock(&swap_status->mutex);
        return NULL;
    }
    
    // Buscamos un bloque libre lo suficientemente grande (best-fit)
    t_swap_block* current = swap_status->first_block;
    t_swap_block* best_block = NULL;
    uint32_t best_size = UINT32_MAX;
    
    while (current != NULL) {
        if (current->pid == 0 && current->num_pages >= num_pages) {
            if (current->num_pages < best_size) {
                best_block = current;
                best_size = current->num_pages;
                
                // Si encontramos un bloque que se ajusta exactamente, terminamos la búsqueda
                if (best_size == num_pages) {
                    break;
                }
            }
        }
        current = current->next;
    }
    
    // Si no encontramos un bloque adecuado, intentamos compactar
    if (best_block == NULL) {
        // Intentamos compactar para reducir fragmentación
        pthread_mutex_unlock(&swap_status->mutex);
        bool compacted = try_compact_swap();
        pthread_mutex_lock(&swap_status->mutex);
        
        if (compacted) {
            // Volvemos a buscar después de compactar
            current = swap_status->first_block;
            best_block = NULL;
            best_size = UINT32_MAX;
            
            while (current != NULL) {
                if (current->pid == 0 && current->num_pages >= num_pages) {
                    if (current->num_pages < best_size) {
                        best_block = current;
                        best_size = current->num_pages;
                        
                        if (best_size == num_pages) {
                            break;
                        }
                    }
                }
                current = current->next;
            }
        }
    }
    
    // Si aún no encontramos un bloque, extendemos el archivo
    if (best_block == NULL) {
        log_info(logger, "No se encontró un bloque adecuado, extendiendo archivo de swap");
        pthread_mutex_unlock(&swap_status->mutex);
        
        if (!extend_swap_file(num_pages)) {
            return NULL;
        }
        
        // Volvemos a intentar la asignación
        return allocate_swap_pages(pid, num_pages);
    }
    
    // Procesamos el bloque encontrado
    t_swap_page_info* result = malloc(sizeof(t_swap_page_info));
    if (result == NULL) {
        log_error(logger, "Error de memoria al asignar t_swap_page_info");
        pthread_mutex_unlock(&swap_status->mutex);
        return NULL;
    }
    
    result->offset = best_block->offset;
    result->pid = pid;
    result->index = 0;  // El índice lo asignará el llamador
    
    // Actualizamos el bloque: si es más grande de lo que necesitamos, lo dividimos
    if (best_block->num_pages > num_pages) {
        // Creamos un nuevo bloque libre con las páginas restantes
        uint32_t new_offset = best_block->offset + (num_pages * swap_status->page_size);
        uint32_t remaining_pages = best_block->num_pages - num_pages;
        
        t_swap_block* new_block = create_block(new_offset, remaining_pages, 0);
        new_block->next = best_block->next;
        new_block->prev = best_block;
        
        if (best_block->next != NULL) {
            best_block->next->prev = new_block;
        }
        
        best_block->next = new_block;
        best_block->num_pages = num_pages;
    }
    
    // Marcamos el bloque como ocupado
    best_block->pid = pid;
    
    // Actualizamos el contador de páginas libres
    swap_status->free_pages -= num_pages;
    
    log_info(logger, "Asignadas %u páginas para proceso %u en offset %u", 
             num_pages, pid, result->offset / swap_status->page_size);
    
    pthread_mutex_unlock(&swap_status->mutex);
    return result;
}

// Lee páginas del archivo de swap
bool read_swap_pages(t_swap_page_info* page_info, uint32_t num_pages, void* buffer) {
    log_info(logger, "Leyendo %u páginas para PID %u desde offset %u", 
             num_pages, page_info->pid, page_info->offset);
    
    pthread_rwlock_rdlock(&swap_status->rwlock);
    
    uint32_t total_size = num_pages * swap_status->page_size;
    ssize_t bytes_read;
    
    // Posicionamos el puntero del archivo
    if (lseek(swap_status->fd, page_info->offset, SEEK_SET) == -1) {
        log_error(logger, "Error al posicionar el puntero del archivo para lectura");
        pthread_rwlock_unlock(&swap_status->rwlock);
        return false;
    }
    
    // Leemos los datos
    bytes_read = read(swap_status->fd, buffer, total_size);
    if (bytes_read != total_size) {
        log_error(logger, "Error al leer datos de swap. Esperados: %u, Leídos: %zd", 
                 total_size, bytes_read);
        pthread_rwlock_unlock(&swap_status->rwlock);
        return false;
    }
    
    log_info(logger, "Lectura exitosa de %u páginas para PID %u", num_pages, page_info->pid);
    
    pthread_rwlock_unlock(&swap_status->rwlock);
    return true;
}

// Escribe páginas en el archivo de swap
bool write_swap_pages(t_swap_page_info* page_info, uint32_t num_pages, void* buffer) {
    log_info(logger, "Escribiendo %u páginas para PID %u en offset %u", 
             num_pages, page_info->pid, page_info->offset);
    
    pthread_rwlock_wrlock(&swap_status->rwlock);
    
    uint32_t total_size = num_pages * swap_status->page_size;
    ssize_t bytes_written;
    
    // Posicionamos el puntero del archivo
    if (lseek(swap_status->fd, page_info->offset, SEEK_SET) == -1) {
        log_error(logger, "Error al posicionar el puntero del archivo para escritura");
        pthread_rwlock_unlock(&swap_status->rwlock);
        return false;
    }
    
    // Escribimos los datos
    bytes_written = write(swap_status->fd, buffer, total_size);
    if (bytes_written != total_size) {
        log_error(logger, "Error al escribir datos en swap. Esperados: %u, Escritos: %zd", 
                 total_size, bytes_written);
        pthread_rwlock_unlock(&swap_status->rwlock);
        return false;
    }
    
    // Forzamos la escritura a disco
    if (fsync(swap_status->fd) == -1) {
        log_error(logger, "Error al forzar la escritura a disco");
        pthread_rwlock_unlock(&swap_status->rwlock);
        return false;
    }
    
    log_info(logger, "Escritura exitosa de %u páginas para PID %u", num_pages, page_info->pid);
    
    pthread_rwlock_unlock(&swap_status->rwlock);
    return true;
}

// Libera páginas de swap
bool free_swap_pages(t_swap_page_info* page_info, uint32_t num_pages) {
    log_info(logger, "Liberando %u páginas para PID %u en offset %u", 
             num_pages, page_info->pid, page_info->offset);
    
    pthread_mutex_lock(&swap_status->mutex);
    
    // Buscamos el bloque correspondiente
    t_swap_block* current = swap_status->first_block;
    while (current != NULL) {
        if (current->offset == page_info->offset && current->pid == page_info->pid) {
            break;
        }
        current = current->next;
    }
    
    // Si no encontramos el bloque, error
    if (current == NULL) {
        log_error(logger, "Error: No se encontró el bloque a liberar para PID %u en offset %u", 
                 page_info->pid, page_info->offset);
        pthread_mutex_unlock(&swap_status->mutex);
        return false;
    }
    
    // Liberamos el bloque
    current->pid = 0;
    swap_status->free_pages += current->num_pages;
    
    // Intentamos fusionar con bloques adyacentes
    merge_adjacent_free_blocks(current);
    
    log_info(logger, "Liberadas %u páginas para PID %u", num_pages, page_info->pid);
    
    pthread_mutex_unlock(&swap_status->mutex);
    free(page_info);
    return true;
}

// Cierra y limpia el sistema de swap
void destroy_swap() {
    if (swap_status == NULL) {
        return;
    }
    
    log_info(logger, "Cerrando sistema de swap");
    
    // Cerramos el archivo
    if (swap_status->fd != -1) {
        close(swap_status->fd);
    }
    
    // Liberamos los bloques
    t_swap_block* current = swap_status->first_block;
    while (current != NULL) {
        t_swap_block* next = current->next;
        free(current);
        current = next;
    }
    
    // Limpiamos recursos
    pthread_mutex_destroy(&swap_status->mutex);
    pthread_rwlock_destroy(&swap_status->rwlock);
    
    free(swap_status->path);
    free(swap_status);
    swap_status = NULL;
    
    log_info(logger, "Sistema de swap cerrado");
}

// Imprime información sobre el estado del swap
void print_swap_status() {
    if (swap_status == NULL) {
        printf("El sistema de swap no está inicializado\n");
        return;
    }
    
    pthread_mutex_lock(&swap_status->mutex);
    
    printf("=== Estado del sistema de swap ===\n");
    printf("Archivo: %s\n", swap_status->path);
    printf("Páginas totales: %u\n", swap_status->total_pages);
    printf("Páginas libres: %u\n", swap_status->free_pages);
    printf("Tamaño de página: %u bytes\n", swap_status->page_size);
    
    printf("\nBloques de swap:\n");
    t_swap_block* current = swap_status->first_block;
    int i = 0;
    while (current != NULL) {
        printf("[%d] Offset: %u, Páginas: %u, PID: %u (%s)\n", 
               i++, current->offset, current->num_pages, current->pid, 
               current->pid == 0 ? "Libre" : "Ocupado");
        current = current->next;
    }
    
    pthread_mutex_unlock(&swap_status->mutex);
}

// Función que simula un proceso usando el sistema de swap
void* process_simulation(void* arg) {
    uint32_t pid = *((uint32_t*)arg);
    uint32_t num_pages = (pid % 3) + 1;  // 1-3 páginas por proceso
    
    log_info(logger, "Proceso %u inicia. Solicitando %u páginas", pid, num_pages);
    
    // Creamos datos para escribir
    char* data = malloc(num_pages * SWAP_PAGE_SIZE);
    for (uint32_t i = 0; i < num_pages * SWAP_PAGE_SIZE; i++) {
        data[i] = 'A' + (pid % 26);
    }
    
    // Asignamos páginas
    t_swap_page_info* page_info = allocate_swap_pages(pid, num_pages);
    if (page_info == NULL) {
        log_error(logger, "Proceso %u: Error al asignar páginas", pid);
        free(data);
        return NULL;
    }
    
    // Escribimos datos
    if (!write_swap_pages(page_info, num_pages, data)) {
        log_error(logger, "Proceso %u: Error al escribir páginas", pid);
        free(data);
        free(page_info);
        return NULL;
    }
    
    // Simulamos algún procesamiento
    usleep(100000 * (pid % 5));
    
    // Creamos buffer para lectura
    char* read_buffer = calloc(1, num_pages * SWAP_PAGE_SIZE);
    
    // Leemos datos
    if (!read_swap_pages(page_info, num_pages, read_buffer)) {
        log_error(logger, "Proceso %u: Error al leer páginas", pid);
        free(data);
        free(read_buffer);
        free(page_info);
        return NULL;
    }
    
    // Verificamos que los datos sean correctos
    bool data_ok = true;
    for (uint32_t i = 0; i < num_pages * SWAP_PAGE_SIZE; i++) {
        if (data[i] != read_buffer[i]) {
            data_ok = false;
            log_error(logger, "Proceso %u: Datos incorrectos en posición %u. Esperado: %c, Leído: %c", 
                     pid, i, data[i], read_buffer[i]);
            break;
        }
    }
    
    if (data_ok) {
        log_info(logger, "Proceso %u: Verificación de datos exitosa", pid);
    }
    
    // Liberamos páginas
    if (!free_swap_pages(page_info, num_pages)) {
        log_error(logger, "Proceso %u: Error al liberar páginas", pid);
    }
    
    free(data);
    free(read_buffer);
    
    log_info(logger, "Proceso %u finalizado", pid);
    return NULL;
}

// Prueba concurrencia en el sistema de swap
void test_concurrency() {
    const int NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
    uint32_t pids[NUM_THREADS];
    
    log_info(logger, "Iniciando prueba de concurrencia con %d procesos", NUM_THREADS);
    
    // Creamos hilos para simular procesos
    for (int i = 0; i < NUM_THREADS; i++) {
        pids[i] = i + 1;  // PIDs 1-10
        if (pthread_create(&threads[i], NULL, process_simulation, &pids[i]) != 0) {
            log_error(logger, "Error al crear hilo para proceso %d", pids[i]);
        }
    }
    
    // Esperamos a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    log_info(logger, "Prueba de concurrencia completada");
}

int main() {
    // Inicializamos el logger
    init_logger();
    log_info(logger, "Iniciando prueba de sistema de swap");
    
    // Inicializamos el sistema de swap
    if (!init_swap()) {
        log_error(logger, "Error al inicializar el sistema de swap");
        log_destroy(logger);
        return 1;
    }
    
    // Mostramos estado inicial
    print_swap_status();
    
    // Prueba simple: asignar, escribir, leer y liberar páginas
    log_info(logger, "\n=== Prueba simple ===");
    
    t_swap_page_info* page_info = allocate_swap_pages(100, 2);
    if (page_info != NULL) {
        char test_data[SWAP_PAGE_SIZE * 2];
        memset(test_data, 'X', sizeof(test_data));
        
        if (write_swap_pages(page_info, 2, test_data)) {
            log_info(logger, "Escritura exitosa");
            
            char read_data[SWAP_PAGE_SIZE * 2];
            memset(read_data, 0, sizeof(read_data));
            
            if (read_swap_pages(page_info, 2, read_data)) {
                log_info(logger, "Lectura exitosa");
                
                bool match = (memcmp(test_data, read_data, sizeof(test_data)) == 0);
                log_info(logger, "Verificación de datos: %s", match ? "OK" : "ERROR");
            }
        }
        
        free_swap_pages(page_info, 2);
    }
    
    // Mostramos estado después de la prueba simple
    print_swap_status();
    
    // Prueba de extensión del archivo: asignar muchas páginas
    log_info(logger, "\n=== Prueba de extensión ===");
    
    t_swap_page_info* large_alloc = allocate_swap_pages(200, 20);
    if (large_alloc != NULL) {
        log_info(logger, "Asignación grande exitosa");
        free_swap_pages(large_alloc, 20);
    }
    
    // Mostramos estado después de la prueba de extensión
    print_swap_status();
    
    // Prueba de fragmentación y compactación
    log_info(logger, "\n=== Prueba de fragmentación y compactación ===");
    
    t_swap_page_info* alloc1 = allocate_swap_pages(301, 2);
    t_swap_page_info* alloc2 = allocate_swap_pages(302, 3);
    t_swap_page_info* alloc3 = allocate_swap_pages(303, 1);
    t_swap_page_info* alloc4 = allocate_swap_pages(304, 4);
    
    // Liberamos algunos bloques para crear fragmentación
    if (alloc1) free_swap_pages(alloc1, 2);
    if (alloc3) free_swap_pages(alloc3, 1);
    
    print_swap_status();
    
    // Forzamos compactación
    log_info(logger, "Forzando compactación...");
    try_compact_swap();
    
    print_swap_status();
    
    // Liberamos los bloques restantes
    if (alloc2) free_swap_pages(alloc2, 3);
    if (alloc4) free_swap_pages(alloc4, 4);
    
    // Prueba de concurrencia
    log_info(logger, "\n=== Prueba de concurrencia ===");
    test_concurrency();
    
    // Estado final
    print_swap_status();
    
    // Limpiamos
    destroy_swap();
    log_info(logger, "Prueba finalizada");
    log_destroy(logger);
    
    return 0;
}
