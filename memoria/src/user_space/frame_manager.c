#include "frame_manager.h"

static void* user_memory_space = NULL;

static uint32_t MEMORY_SIZE = 0;
static uint32_t BLOCK_SIZE = 0;
static uint32_t MAX_FRAMES = 0;

static char* frame_bitmap = NULL;
static _Atomic uint32_t free_frames_count = 0;
static size_t total_frames_count = 0;

//-------------------------Funciones auxiliares---------------------------------------------
void free_frame_and_uint32(void* element_ptr) {
    uint32_t frame_num = *((uint32_t*)element_ptr);
    if (frame_num < total_frames_count) {
        frame_bitmap[frame_num] = false; // Mark as free
        // Don't increment current_free_frames_count here; recalculate once at the end
    } else {
        LOG_WARNING("Frame Manager: Intentando liberar frame invalido %u.", frame_num);
    }
    free(element_ptr); // Free the uint32_t* itself
}

//----------------------------------------------------------------------------------------

void init_user_memory(const t_memoria_config* config) {
    if (config == NULL) {
        LOG_ERROR("Frame Manger: Configuración de memoria es NULL.");
        exit(1);
    }

    MEMORY_SIZE = config->TAM_MEMORIA;
    BLOCK_SIZE = config->TAM_PAGINA;
    MAX_FRAMES = MEMORY_SIZE / BLOCK_SIZE;

    frame_bitmap = (char*)safe_calloc(MAX_FRAMES, sizeof(char));

    if (MEMORY_SIZE == 0 || BLOCK_SIZE == 0 || MAX_FRAMES == 0) {
        LOG_ERROR("Frame Manger: Tamaño de memoria o tamaño de página inválido. Memoria: %zu, Página: %d, Máximo Frames: %d", MEMORY_SIZE, BLOCK_SIZE, MAX_FRAMES);
        exit(1);
    }

    user_memory_space = safe_calloc(MEMORY_SIZE, 1);

    LOG_INFO("Memoria de usuario inicializada con tamaño %zu bytes y tamaño de página %d bytes.", MEMORY_SIZE, BLOCK_SIZE);

}

void frame_allocation_init() {
    if (BLOCK_SIZE == 0) {
        LOG_ERROR("Frame Manager: PAGE_SIZE no inicializado. Llama a init_user_memory primero.");
        return;
    }

    total_frames_count = (size_t)ceil((double)MEMORY_SIZE / BLOCK_SIZE);
    frame_bitmap = (bool*)malloc(sizeof(bool) * total_frames_count);
    if (frame_bitmap == NULL) {
        LOG_ERROR("Frame Manager: Error al asignar bitmap de frames.");
        exit(EXIT_FAILURE);
    }
    //Inicializamos todos los frames como libres
    for (size_t i = 0; i < total_frames_count; i++) {
        frame_bitmap[i] = false;
    }
    free_frames_count = total_frames_count;

    LOG_INFO("Frame Manager: Inicializado con %zu frames disponibles.", total_frames_count);
}

t_list* frame_allocate_frames(uint32_t num_frames) {
    if (num_frames == 0) {
        LOG_WARNING("Frame Manager: Solicitud de asignación de frames con número cero. Se ignorará la solicitud.");
        return NULL;
    }

    lock_frame_manager();

    if (free_frames_count < num_frames)
    {
        LOG_WARNING("Frame Manager: No hay suficientes frames libres para asignar %u frames. Libre: %u.", num_frames, free_frames_count);
        unlock_frame_manager();
        return NULL;
    }

    t_list* allocated_list = list_create();
    if (allocated_list == NULL) {
        LOG_ERROR("Frame Manager: Error al crear lista para frames asignados.");
        unlock_frame_manager();
        return NULL;
    }

    uint32_t allocated_count = 0;
    for (uint32_t i = 0; i < total_frames_count && allocated_count < num_frames; i++) {
        if (!frame_bitmap[i]) { // If frame is free
            frame_bitmap[i] = true; // Mark as occupied
            uint32_t* frame_num = safe_malloc(sizeof(uint32_t));
            *frame_num = i;
            list_add(allocated_list, frame_num);
            allocated_count++;
        }
    }

    free_frames_count -= num_frames;
    LOG_INFO("Frame Manager: Asignados %u frames. Frames libres restantes: %u.", num_frames, free_frames_count);
    unlock_frame_manager();
    return allocated_list;
}

void frame_free_frames(t_list* frame_numbers_list) {
    if (frame_numbers_list == NULL)
    {
        LOG_WARNING("Frame Manager: Intento de liberar frames con lista nula. No se realizará ninguna acción.");   
        return;
    }

    lock_frame_manager();

    list_destroy_and_destroy_elements(frame_numbers_list, free_frame_and_uint32);

    //  Recalcular el numero de frames libres
    uint32_t new_free_count = 0;
    for (uint32_t i = 0; i < total_frames_count; i++) {
        if (!frame_bitmap[i]) {
            new_free_count++;
        }
    }
    free_frames_count = new_free_count;

    LOG_INFO("Frame Manager: Frames liberados. Frames libres restantes: %d.", free_frames_count);
    unlock_frame_manager();
}


uint32_t frame_get_free_count() {
    return free_frames_count;
    // No lo uso lock porque lo declare Atomic
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
        LOG_ERROR("Intento de escritura con puntero nulo o tamanio cero.");
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
    if (BLOCK_SIZE == 0) {
        LOG_ERROR("Tamaño de página no inicializado para lectura de página completa.");
        return false;
    }
    if (physical_address % BLOCK_SIZE != 0) {
        LOG_ERROR("Dirección física %u no alineada a página para lectura completa. Tamaño página: %d", physical_address, BLOCK_SIZE);
        return false;
    }
    return read_memory(physical_address, buffer, BLOCK_SIZE);
}

bool update_full_page(uint32_t physical_address, const void* data) {
    if (BLOCK_SIZE == 0) {
        LOG_ERROR("Tamaño de página no inicializado para actualización de página completa.");
        return false;
    }
    if (physical_address % BLOCK_SIZE != 0) {
        LOG_ERROR("Dirección física %u no alineada a página para actualización completa. Tamaño página: %d", physical_address, BLOCK_SIZE);
        return false;
    }
    return write_memory(physical_address, data, BLOCK_SIZE);
}

void memory_manager_destroy() {
    if (frame_bitmap != NULL) {
        free(frame_bitmap);
        frame_bitmap = NULL;
    }
    if (user_memory_space != NULL) {
        free(user_memory_space);
        user_memory_space = NULL;
        LOG_INFO("Memoria de usuario y Frame Manager liberados.");
    }
}