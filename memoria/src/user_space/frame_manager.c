#include "frame_manager.h"

void* user_memory_space = NULL;
size_t MEMORY_SIZE = 0;
int PAGE_SIZE = 0;

static bool* frame_bitmap = NULL;
static size_t total_frames_count = 0;
static size_t current_free_frames_count = 0;

// Helper to free uint32_t* elements in lists
static void free_uint32_ptr(void* element) {
    free(element);
}

bool init_user_memory(const t_memoria_config* config) {
    if (config == NULL) {
        LOG_ERROR("Configuración de memoria es NULL.");
        return false;
    }

    MEMORY_SIZE = config->TAM_MEMORIA;
    PAGE_SIZE = config->TAM_PAGINA;

    user_memory_space = malloc(MEMORY_SIZE);
    if (user_memory_space == NULL) {
        LOG_ERROR("Error al asignar memoria de usuario de tamaño %zu.", MEMORY_SIZE);
        return false;
    }

    memset(user_memory_space, 0, MEMORY_SIZE);

    LOG_INFO("Memoria de usuario inicializada con tamaño %zu bytes y tamaño de página %d bytes.", MEMORY_SIZE, PAGE_SIZE);

    return true;
}

void frame_allocation_init() {
    if (PAGE_SIZE == 0) {
        LOG_ERROR("Frame Manager: PAGE_SIZE no inicializado. Llama a init_user_memory primero.");
        return;
    }

    total_frames_count = (size_t)ceil((double)MEMORY_SIZE / PAGE_SIZE);
    frame_bitmap = (bool*)malloc(sizeof(bool) * total_frames_count);
    if (frame_bitmap == NULL) {
        LOG_ERROR("Frame Manager: Error al asignar bitmap de frames.");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < total_frames_count; i++) {
        frame_bitmap[i] = false; // false means free
    }
    current_free_frames_count = total_frames_count;

    LOG_INFO("Frame Manager: Inicializado con %zu frames disponibles.", total_frames_count);
}


t_list* frame_allocate_frames(size_t num_frames) {
    if (num_frames == 0) return list_create();

    lock_frame_manager();

    if (current_free_frames_count < num_frames) {
        LOG_WARNING("Frame Manager: No hay suficientes frames libres para asignar %zu frames. Libre: %zu.", num_frames, current_free_frames_count);
        unlock_frame_manager();
        return NULL;
    }

    t_list* allocated_list = list_create();
    if (allocated_list == NULL) {
        LOG_ERROR("Frame Manager: Error al crear lista para frames asignados.");
        unlock_frame_manager();
        return NULL;
    }

    size_t allocated_count = 0;
    for (size_t i = 0; i < total_frames_count && allocated_count < num_frames; i++) {
        if (!frame_bitmap[i]) { // If frame is free
            frame_bitmap[i] = true; // Mark as occupied
            uint32_t* frame_num = malloc(sizeof(uint32_t));
            if (frame_num == NULL) {
                LOG_ERROR("Frame Manager: Error al asignar memoria para numero de frame.");
                list_destroy_and_destroy_elements(allocated_list, free_uint32_ptr);
                // Manually revert bitmap for frames that were marked true
                for (size_t j = 0; j < i; j++) {
                    if (frame_bitmap[j]) { // Only if it was marked true by this allocation attempt
                        bool found_in_list = false;
                        for (int k = 0; k < list_size(allocated_list); k++) {
                            if (*((uint32_t*)list_get(allocated_list, k)) == j) {
                                found_in_list = true;
                                break;
                            }
                        }
                        if (found_in_list) frame_bitmap[j] = false;
                    }
                }
                unlock_frame_manager();
                return NULL;
            }
            *frame_num = (uint32_t)i;
            list_add(allocated_list, frame_num);
            allocated_count++;
        }
    }

    if (allocated_count == num_frames) {
        current_free_frames_count -= num_frames;
        LOG_INFO("Frame Manager: Asignados %zu frames. Frames libres restantes: %zu.", num_frames, current_free_frames_count);
        unlock_frame_manager();
        return allocated_list;
    } else {
        LOG_ERROR("Frame Manager: Fallo al asignar %zu frames. Solo se pudieron asignar %zu.", num_frames, allocated_count);
        // If partial allocation, free what was allocated and clean up
        list_destroy_and_destroy_elements(allocated_list, free_uint32_ptr);
        // Correct the bitmap if there was a partial allocation failure
        for (size_t i = 0; i < total_frames_count; i++) {
            if (frame_bitmap[i]) {
                bool found_in_current_list = false; // Check if frame 'i' was among the *intended* allocation list
                for (int k = 0; k < list_size(allocated_list); k++) {
                    if (*((uint32_t*)list_get(allocated_list, k)) == i) {
                        found_in_current_list = true;
                        break;
                    }
                }
                if (found_in_current_list) frame_bitmap[i] = false; // Revert only those marked by this failed attempt
            }
        }
        current_free_frames_count = total_frames_count - allocated_count; // This is a rough fix, full bitmap scan is safer.
        unlock_frame_manager();
        return NULL;
    }
}

void frame_free_frames(t_list* frame_numbers_list) {
    if (frame_numbers_list == NULL) return;

    lock_frame_manager();
    void _free_frame_and_uint32(void* element_ptr) {
        uint32_t frame_num = *((uint32_t*)element_ptr);
        if (frame_num < total_frames_count) {
            frame_bitmap[frame_num] = false; // Mark as free
            // Don't increment current_free_frames_count here; recalculate once at the end
        } else {
            LOG_WARNING("Frame Manager: Intentando liberar frame invalido %u.", frame_num);
        }
        free(element_ptr); // Free the uint32_t* itself
    }

    list_destroy_and_destroy_elements(frame_numbers_list, _free_frame_and_uint32);

    // Recalculate free frames by iterating bitmap after all frees for this list are done
    size_t new_free_count = 0;
    for (size_t i = 0; i < total_frames_count; i++) {
        if (!frame_bitmap[i]) {
            new_free_count++;
        }
    }
    current_free_frames_count = new_free_count;

    LOG_INFO("Frame Manager: Frames liberados. Frames libres restantes: %zu.", current_free_frames_count);
    unlock_frame_manager();
}


size_t frame_get_free_count() {
    lock_frame_manager();
    size_t count = current_free_frames_count;
    unlock_frame_manager();
    return count;
}

bool read_memory(uint32_t physical_address, void* buffer, size_t size) {
    if (user_memory_space == NULL || buffer == NULL || size == 0) {
        LOG_ERROR("Intento de lectura con puntero nulo o tamaño cero.");
        return false;
    }

    if (physical_address + size > MEMORY_SIZE) {
        LOG_ERROR("Intento de lectura fuera de límites de memoria. Dir: %u, Tamaño: %zu, Memoria Total: %zu", physical_address, size, MEMORY_SIZE);
        return false;
    }

    memcpy(buffer, (char*)user_memory_space + physical_address, size);
    return true;
}

bool write_memory(uint32_t physical_address, const void* data, size_t size) {
    if (user_memory_space == NULL || data == NULL || size == 0) {
        LOG_ERROR("Intento de escritura con puntero nulo o tamaño cero.");
        return false;
    }

    if (physical_address + size > MEMORY_SIZE) {
        LOG_ERROR("Intento de escritura fuera de límites de memoria. Dir: %u, Tamaño: %zu, Memoria Total: %zu", physical_address, size, MEMORY_SIZE);
        return false;
    }

    memcpy((char*)user_memory_space + physical_address, data, size);
    return true;
}

bool read_full_page(uint32_t physical_address, void* buffer) {
    if (PAGE_SIZE == 0) {
        LOG_ERROR("Tamaño de página no inicializado para lectura de página completa.");
        return false;
    }
    if (physical_address % PAGE_SIZE != 0) {
        LOG_ERROR("Dirección física %u no alineada a página para lectura completa. Tamaño página: %d", physical_address, PAGE_SIZE);
        return false;
    }
    return read_memory(physical_address, buffer, PAGE_SIZE);
}

bool update_full_page(uint32_t physical_address, const void* data) {
    if (PAGE_SIZE == 0) {
        LOG_ERROR("Tamaño de página no inicializado para actualización de página completa.");
        return false;
    }
    if (physical_address % PAGE_SIZE != 0) {
        LOG_ERROR("Dirección física %u no alineada a página para actualización completa. Tamaño página: %d", physical_address, PAGE_SIZE);
        return false;
    }
    return write_memory(physical_address, data, PAGE_SIZE);
}

void memory_manager_destroy() {
    if (frame_bitmap != NULL) {
        free(frame_bitmap);
        frame_bitmap = NULL;
    }
    if (user_memory_space != NULL) {
        free(user_memory_space);
        user_memory_space = NULL;
        MEMORY_SIZE = 0;
        PAGE_SIZE = 0;
        LOG_INFO("Memoria de usuario y Frame Manager liberados.");
    }
}