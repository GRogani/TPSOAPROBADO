#include "manager.h" 

void* user_memory_space = NULL;
size_t MEMORY_SIZE = 0;
int PAGE_SIZE = 0;

bool init_user_memory(const t_memoria_config* config) {
    if (config == NULL) {
        LOG_ERROR("Configuración de memoria es NULL.");
        return false;
    }

    MEMORY_SIZE = config->TAM_MEMORIA;
    PAGE_SIZE = config->TAM_PAGINA;

    user_memory_space = safe_malloc(MEMORY_SIZE);
    if (user_memory_space == NULL) {
        LOG_ERROR("Error al asignar memoria de usuario de tamaño %zu.", MEMORY_SIZE);
        return false;
    }

    memset(user_memory_space, 0, MEMORY_SIZE);

    LOG_INFO("Memoria de usuario inicializada con tamaño %zu bytes y tamaño de página %d bytes.", MEMORY_SIZE, PAGE_SIZE);

    return true;
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
    LOG_INFO("Lectura de %zu bytes en dir. física %u exitosa.", size, physical_address); // Too verbose for frequent calls
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
    LOG_INFO("Escritura de %zu bytes en dir. física %u exitosa.", size, physical_address); // Too verbose for frequent calls
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
    if (user_memory_space != NULL) {
        free(user_memory_space);
        user_memory_space = NULL;
        MEMORY_SIZE = 0;
        PAGE_SIZE = 0;
        LOG_INFO("Memoria de usuario liberada.");
    }
}