#include "safe_alloc.h"

void* safe_malloc(size_t size) {
    void* pointer = malloc(size);
    if (pointer == NULL) {
        LOG_ERROR("No se pudo asignar memoria de %zu bytes.", size);
        exit(EXIT_FAILURE);
    }
    return pointer;
}

void* safe_realloc(void* pointer, size_t size) {
    void* new_pointer = realloc(pointer, size);
    if (new_pointer == NULL) {
        LOG_ERROR("No se pudo reasignar memoria de %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return new_pointer;
}