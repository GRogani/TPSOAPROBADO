#ifndef SAFE_ALLOC_H
#define SAFE_ALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include "log_error_macro.h"

/**
 * @brief Intenta asignar memoria dinámica de forma segura.
 * 
 * Esta función encapsula malloc() y verifica si la asignación fue exitosa.
 * Si falla, imprime un mensaje de error en stderr y finaliza el programa.
 * 
 * @param size Tamaño en bytes a asignar.
 * @return void* Puntero a la memoria asignada.
 */
void* safe_malloc(size_t size);

/**
 * @brief Intenta reasignar memoria dinámica de forma segura.
 * 
 * Esta función encapsula `realloc()` y verifica si la reasignación fue exitosa.
 * Si falla, imprime un mensaje de error en `stderr` y finaliza el programa.
 * 
 * @param pointer Puntero a la memoria previamente asignada que se desea redimensionar.
 * @param size Tamaño en bytes a reasignar.
 * @return void* Puntero a la nueva memoria reasignada.
 */
void* safe_realloc(void* pointer, size_t size);


#endif