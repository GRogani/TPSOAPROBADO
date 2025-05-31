#ifndef UTLIS_SAFE_ALLOC_H
#define UTILS_SAFE_ALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include "../macros/logger_macro.h"

/**
 * @brief Intenta asignar memoria dinámica de forma segura.
 * 
 * Esta función encapsula malloc() y verifica si la asignación fue exitosa.
 * Si falla, imprime un mensaje de error en stderr y sale con `abort()`.
 * 
 * @note `abort()` genera una `SIGABRT` que puede ser catcheada por `signal(...)`
 * 
 * @param size Tamaño en bytes a asignar.
 * @return void* Puntero a la memoria asignada.
 */
void* safe_malloc(size_t size);

/**
 * @brief Intenta reasignar memoria dinámica de forma segura.
 * 
 * Esta función encapsula `realloc()` y verifica si la reasignación fue exitosa.
 * Si falla, imprime un mensaje de error en `stderr` y aborta.
 * 
 * @param pointer Puntero a la memoria previamente asignada que se desea redimensionar.
 * @param size Tamaño en bytes a reasignar.
 * @return void* Puntero a la nueva memoria reasignada.
 */
void* safe_realloc(void* pointer, size_t size);

/**
 * @brief Intenta asignar memoria dinámica inicializada en cero de forma segura.
 * 
 * Esta función encapsula `calloc()` y verifica si la asignación fue exitosa.
 * Si falla, imprime un mensaje de error en `stderr` y aborta.
 * 
 * @param count Número de elementos a asignar.
 * @param size Tamaño en bytes de cada elemento.
 * @return void* Puntero a la memoria asignada.
 */
void* safe_calloc(size_t count, size_t size);

#endif