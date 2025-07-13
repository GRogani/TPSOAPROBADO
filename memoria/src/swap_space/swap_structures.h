#ifndef SWAP_SPACE_SWAP_STRUCTURES_H
#define SWAP_SPACE_SWAP_STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "../../../utils/utils.h"

/**
 * @brief Estado del swap para mostrar métricas y diagnóstico
 */
typedef struct {
    _Atomic uint32_t total_pages;            /** @brief Total de páginas disponibles en el archivo swap */
    _Atomic uint32_t used_pages;             /** @brief Páginas actualmente en uso */
    _Atomic uint32_t read_operations;        /** @brief Número de operaciones de lectura realizadas */
    _Atomic uint32_t write_operations;       /** @brief Número de operaciones de escritura realizadas */
    _Atomic uint32_t allocated_processes;    /** @brief Número de procesos con asignaciones en swap */
} t_swap_status;

/**
 * @brief Estructura para gestionar un bloque de páginas en el archivo de swap
 */
typedef struct {
    uint32_t pid;                 /** @brief PID del proceso dueño */
    uint32_t start_offset;        /** @brief Offset de inicio del bloque en bytes */
    uint32_t num_pages;           /** @brief Número de páginas del bloque */
    bool is_used;                 /** @brief Indica si el bloque está en uso o no */
    struct timespec last_access;  /** @brief Última vez que se accedió al bloque */
} t_swap_block;

/**
 * @brief Estructura para almacenar la información de una página en swap
 */
typedef struct {
    uint32_t pid;                  /** @brief PID del proceso dueño */
    uint32_t virtual_page_number;  /** @brief Número de página virtual en el proceso */
    uint32_t swap_offset;          /** @brief Offset en bytes dentro del archivo swap */
} t_swap_page_info;

#endif // SWAP_SPACE_SWAP_STRUCTURES_H