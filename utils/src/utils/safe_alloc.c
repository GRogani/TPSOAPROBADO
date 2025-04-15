#include "safe_alloc.h"

void* safe_malloc(size_t size) {
    void* pointer = malloc(size);
    if (pointer == NULL) {
        LOG_ERROR("No se pudo asignar memoria de %zu bytes.", size);
        abort();
    }
    return pointer;
}

void* safe_realloc(void* pointer, size_t size) {
    void* new_pointer = realloc(pointer, size);
    if (new_pointer == NULL) {
        LOG_ERROR("No se pudo reasignar memoria de %zu bytes.\n", size);
        abort();
    }
    return new_pointer;
}

void* safe_calloc(size_t count, size_t size) {
    void* pointer = calloc(count, size);
    if (pointer == NULL) {
        LOG_ERROR("No se pudo asignar memoria con calloc (%zu elementos de %zu bytes).", count, size);
        abort();
    }
    return pointer;
}