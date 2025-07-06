#include "swap_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static FILE* swap_file_ptr = NULL;
static char* swap_file_path = NULL;
static int PAGE_SIZE_SWAP = 0;

static t_list* swap_free_blocks = NULL;
static size_t swap_current_free_pages = 0; // Este contador se actualiza al asignar/liberar

/**
 * @brief Libera un elemento t_swap_page_info
 * @param element Puntero al elemento a liberar
 */
void free_swap_page_info_element(void* element) {
    t_swap_page_info* info = (t_swap_page_info*)element;
    free(info);
}

/**
 * @brief Libera un bloque libre de swap
 * @param element Puntero al bloque a liberar
 */
void free_swap_free_block(void* element) {
    t_swap_free_block* block = (t_swap_free_block*)element;
    free(block);
}

/**
 * @brief Compara dos bloques libres por offset de inicio
 * @param a Primer bloque
 * @param b Segundo bloque
 * @return true si a < b, false en caso contrario
 */
bool compare_swap_free_blocks(void* a, void* b) {
    t_swap_free_block* block_a = (t_swap_free_block*)a;
    t_swap_free_block* block_b = (t_swap_free_block*)b;
    return block_a->start_offset < block_b->start_offset;
}

/**
 * @brief Fusiona bloques libres adyacentes
 */
void merge_free_blocks() {
    if (swap_free_blocks == NULL || list_size(swap_free_blocks) < 2) {
        return;
    }

    list_sort(swap_free_blocks, (void*) &compare_swap_free_blocks);

    int i = 0;
    while (i < list_size(swap_free_blocks) - 1) {
        t_swap_free_block* current = list_get(swap_free_blocks, i);
        t_swap_free_block* next = list_get(swap_free_blocks, i + 1);

        if (current->start_offset + current->size == next->start_offset) {
            current->size += next->size;
            list_remove_and_destroy_element(swap_free_blocks, i + 1, free_swap_free_block);
        } else {
            i++;
        }
    }
}

// --- Funciones Públicas del Swap Manager ---

bool swap_manager_init(const t_memoria_config* config) {
    if (config == NULL || config->TAM_PAGINA == 0 || config->PATH_SWAPFILE == NULL) {
        LOG_ERROR("Swap Manager: Configuracion invalida para inicializacion (TAM_PAGINA o PATH_SWAPFILE missing).");
        return false;
    }

    swap_file_path = strdup(config->PATH_SWAPFILE);
    if (swap_file_path == NULL) {
        LOG_ERROR("Swap Manager: Error al duplicar PATH_SWAPFILE.");
        return false;
    }

    PAGE_SIZE_SWAP = config->TAM_PAGINA;

    // Abrir el archivo en modo binario de lectura/escritura. "r+b" para existente, "w+b" para crear/truncar.
    // Queremos persistir el archivo entre ejecuciones, así que intentamos "r+b" primero.
    swap_file_ptr = fopen(swap_file_path, "r+b");
    if (swap_file_ptr == NULL) {
        // Si el archivo no existe, crearlo.
        swap_file_ptr = fopen(swap_file_path, "w+b");
        if (swap_file_ptr == NULL) {
            LOG_ERROR("Swap Manager: No se pudo abrir/crear el archivo SWAP. Verifique la ruta y permisos.");
            free(swap_file_path);
            return false;
        }
    }

    // No hay ftruncate inicial o tamaño predefinido. El archivo crecerá según sea necesario.

    // Inicializar la lista de bloques libres
    if (swap_free_blocks == NULL) {
        swap_free_blocks = list_create();
        if (swap_free_blocks == NULL) {
            LOG_ERROR("Error: No se pudo crear lista de bloques libres de swap");
            return false;
        }
    }

    t_swap_free_block* initial_block = malloc(sizeof(t_swap_free_block));
    if (initial_block == NULL) {
        LOG_ERROR("Error: No se pudo asignar memoria para bloque inicial de swap");
        return false;
    }

    fseek(swap_file_ptr, 0, SEEK_END);
    long file_size = ftell(swap_file_ptr);
    fseek(swap_file_ptr, 0, SEEK_SET);

    initial_block->start_offset = 0;
    initial_block->size = file_size;
    list_add(swap_free_blocks, initial_block);
    swap_current_free_pages = file_size / PAGE_SIZE_SWAP;

    LOG_INFO("Swap Manager: Inicializado con %zu páginas libres", swap_current_free_pages);
    return true;
}

void swap_manager_destroy() {
    if (swap_file_ptr != NULL) {
        fclose(swap_file_ptr);
        swap_file_ptr = NULL;
    }
    if (swap_file_path != NULL) {
        free(swap_file_path);
        swap_file_path = NULL;
    }
    if (swap_free_blocks != NULL) {
        list_destroy_and_destroy_elements(swap_free_blocks, free_swap_free_block);
        swap_free_blocks = NULL;
    }
    LOG_INFO("Swap Manager: Destruido y archivo SWAP cerrado.");
}

t_list* swap_allocate_pages(uint32_t pid, size_t num_pages_to_swap) {
    if (num_pages_to_swap == 0) return list_create();

    lock_swap_file(); // Protege el acceso a las estructuras del SWAP

    t_list* allocated_swap_pages_info = list_create();
    if (allocated_swap_pages_info == NULL) {
        LOG_ERROR("Swap Manager: Error al crear lista de paginas SWAP asignadas.");
        unlock_swap_file();
        return NULL;
    }

    size_t pages_allocated_count = 0;
    // size_t required_bytes = num_pages_to_swap * PAGE_SIZE_SWAP; // No usado directamente, pero útil para entender

    // Intentar asignar desde bloques libres primero (first-fit)
    for (int i = 0; i < list_size(swap_free_blocks) && pages_allocated_count < num_pages_to_swap; i++) {
        t_swap_free_block* current_free_block = list_get(swap_free_blocks, i);

        size_t available_pages_in_block = current_free_block->size / PAGE_SIZE_SWAP;
        size_t pages_to_take = num_pages_to_swap - pages_allocated_count;
        if (pages_to_take > available_pages_in_block) {
            pages_to_take = available_pages_in_block;
        }

        if (pages_to_take > 0) {
            for (size_t j = 0; j < pages_to_take; j++) {
                t_swap_page_info* info = safe_malloc(sizeof(t_swap_page_info)); // Usar safe_malloc
                info->pid = pid;
                // Asignar virtual_page_number aquí es complicado ya que no lo sabemos.
                // Es mejor que el caller lo complete después si lo necesita para la info de proceso.
                // info->virtual_page_number = ???;
                info->swap_offset = current_free_block->start_offset + (j * PAGE_SIZE_SWAP);
                list_add(allocated_swap_pages_info, info);
                pages_allocated_count++;
            }

            current_free_block->start_offset += (pages_to_take * PAGE_SIZE_SWAP);
            current_free_block->size -= (pages_to_take * PAGE_SIZE_SWAP);
            swap_current_free_pages -= pages_to_take;

            if (current_free_block->size == 0) {
                list_remove_and_destroy_element(swap_free_blocks, i, free_swap_free_block);
                i--; // Decrementar 'i' porque se eliminó un elemento y el siguiente se movió a su posición actual
            }
        }
    }

    // Si aún se necesitan páginas, extender el archivo SWAP
    if (pages_allocated_count < num_pages_to_swap) {
        // Mover el puntero al final del archivo para obtener el offset de expansión
        fseek(swap_file_ptr, 0, SEEK_END);
        long current_end_offset = ftell(swap_file_ptr);
        size_t remaining_pages = num_pages_to_swap - pages_allocated_count;

        // Podríamos usar ftruncate para pre-asignar el espacio, pero el enfoque actual
        // de simplemente agregar páginas es funcional si se escriben inmediatamente.
        // Si ftruncate se usa para pre-asignar un bloque grande, se deberia agregar
        // un solo bloque libre grande a swap_free_blocks en lugar de expandir page by page.

        for (size_t j = 0; j < remaining_pages; j++) {
            t_swap_page_info* info = safe_malloc(sizeof(t_swap_page_info));
            info->pid = pid;
            // info->virtual_page_number = ???;
            info->swap_offset = current_end_offset + (j * PAGE_SIZE_SWAP);
            list_add(allocated_swap_pages_info, info);
            pages_allocated_count++;
        }
    }

    if (pages_allocated_count == num_pages_to_swap) {
        LOG_INFO("Swap Manager: Asignadas %zu paginas en SWAP para PID %u. Paginas libres reutilizables: %zu.",
                 num_pages_to_swap, pid, swap_current_free_pages);
        unlock_swap_file();
        return allocated_swap_pages_info;
    } else {
        LOG_ERROR("Swap Manager: Fallo total o parcial al asignar las paginas en SWAP. No se pudo obtener el numero de paginas solicitado.");
        
        // Iterar para liberar cada t_swap_page_info* y ajustar el contador y bloques libres.
        // Esto es similar a lo que hace swap_free_pages pero se hace aquí para evitar re-entrancia de lock.
        void _rollback_allocation(void* element) {
            t_swap_page_info* info_to_free = (t_swap_page_info*)element;
            t_swap_free_block* new_free_block = safe_malloc(sizeof(t_swap_free_block));
            new_free_block->start_offset = info_to_free->swap_offset;
            new_free_block->size = PAGE_SIZE_SWAP;
            list_add(swap_free_blocks, new_free_block);
            swap_current_free_pages++;
            free(info_to_free);
        }
        list_iterate(allocated_swap_pages_info, _rollback_allocation);
        list_destroy(allocated_swap_pages_info); // Destruye la lista vacía

        merge_free_blocks(); // Intenta consolidar los bloques libres que se acaban de crear

        unlock_swap_file();
        return NULL;
    }
}

bool swap_write_page(uint32_t swap_offset, const void* page_content, size_t page_size) {
    // Se asume que lock_swap_file() es llamado por la capa superior (e.g., process_manager)
    // para proteger las operaciones de lectura/escritura del archivo.
    if (swap_file_ptr == NULL || page_content == NULL || page_size == 0) {
        LOG_ERROR("Swap Manager: Intento de escritura invalido (puntero nulo o tamano cero).");
        return false;
    }

    // No se toma el lock aquí, se espera que el caller lo haga.
    // lock_swap_file();
    fseek(swap_file_ptr, swap_offset, SEEK_SET);
    size_t written_bytes = fwrite(page_content, 1, page_size, swap_file_ptr);
    // unlock_swap_file();

    if (written_bytes != page_size) {
        LOG_ERROR("Swap Manager: Error de escritura en SWAP. La cantidad de bytes escritos no coincide con lo esperado.");
        return false;
    }
    LOG_DEBUG("Swap Manager: Pagina escrita en SWAP en offset %u.", swap_offset);
    return true;
}

bool swap_read_page(uint32_t swap_offset, void* buffer, size_t page_size) {
    // Se asume que lock_swap_file() es llamado por la capa superior (e.g., process_manager)
    // para proteger las operaciones de lectura/escritura del archivo.
    if (swap_file_ptr == NULL || buffer == NULL || page_size == 0) {
        LOG_ERROR("Swap Manager: Intento de lectura invalido (puntero nulo o tamano cero).");
        return false;
    }

    // No se toma el lock aquí, se espera que el caller lo haga.
    // lock_swap_file();
    fseek(swap_file_ptr, 0, SEEK_END);
    long file_size = ftell(swap_file_ptr);
    fseek(swap_file_ptr, swap_offset, SEEK_SET); // Volver al offset original

    if (swap_offset + page_size > (uint32_t)file_size) { // Cast file_size to uint32_t for comparison
        LOG_ERROR("Swap Manager: Intento de lectura fuera de los limites del archivo SWAP. El offset y tamano exceden el archivo.");
        // unlock_swap_file(); // Si el caller maneja el lock, no desbloquear aquí
        return false;
    }
    
    size_t read_bytes = fread(buffer, 1, page_size, swap_file_ptr);
    // unlock_swap_file(); // Si el caller maneja el lock, no desbloquear aquí

    if (read_bytes != page_size) {
        LOG_ERROR("Swap Manager: Error de lectura en SWAP. La cantidad de bytes leidos no coincide con lo esperado.");
        return false;
    }
    LOG_DEBUG("Swap Manager: Pagina leida de SWAP desde offset %u.", swap_offset);
    return true;
}

void swap_free_pages(t_list* swap_page_info_list, size_t page_size) {
    if (swap_page_info_list == NULL || list_is_empty(swap_page_info_list)) {
        LOG_DEBUG("Swap Manager: Lista de paginas a liberar de SWAP es nula o vacia. No se hizo nada.");
        return;
    }

    lock_swap_file(); // Protege las operaciones de liberación y el bitmap de bloques libres

    void _free_single_swap_page_info(void* element) {
        t_swap_page_info* info = (t_swap_page_info*)element;
        if (info == NULL) {
            LOG_WARNING("Swap Manager: Elemento nulo encontrado en lista de paginas a liberar.");
            return;
        }

        t_swap_free_block* new_free_block = safe_malloc(sizeof(t_swap_free_block));
        new_free_block->start_offset = info->swap_offset;
        new_free_block->size = page_size;
        list_add(swap_free_blocks, new_free_block);

        swap_current_free_pages++;

        LOG_DEBUG("Swap Manager: Pagina liberada de SWAP en offset %u (PID: %u).",
                  info->swap_offset, info->pid);
        free(info); // Liberar la estructura t_swap_page_info*
    }

    list_iterate(swap_page_info_list, _free_single_swap_page_info);

    // No se usa list_destroy_and_destroy_elements directamente aquí para evitar doble free de la lista
    // ya que la iteración ya liberó los elementos. Solo destruir la lista.
    list_destroy(swap_page_info_list);

    // Fusionar bloques libres adyacentes después de que todas las liberaciones estén hechas
    merge_free_blocks();

    LOG_INFO("Swap Manager: Paginas SWAP liberadas. Paginas libres reutilizables: %zu.", swap_current_free_pages);

    unlock_swap_file(); // Desbloquea el acceso al archivo SWAP
}

size_t swap_get_free_pages_count() {
    lock_swap_file();
    size_t count = swap_current_free_pages;
    unlock_swap_file();
    return count;
}