#ifndef SWAP_SPACE_SWAP_STRUCTURES_H
#define SWAP_SPACE_SWAP_STRUCTURES_H

/**
 * @brief Estructura para almacenar la información de una página que reside en SWAP.
 * Esta estructura se usará para rastrear la ubicación de las páginas de un proceso
 * cuando han sido swapeadas.
 */
typedef struct {
    uint32_t pid;                 /** @brief PID del proceso al que pertenece la página. */
    uint32_t virtual_page_number; /** @brief Número de página virtual dentro del proceso. */
    uint32_t swap_offset;         /** @brief Offset dentro del archivo de SWAP donde está almacenada la página. */
} t_swap_page_info;

/**
 * @brief Estructura para representar un bloque de proceso en el archivo de SWAP.
 * Simplifica la gestión al manejar procesos completos en lugar de páginas individuales.
 */
typedef struct {
    uint32_t pid;                 /** @brief ID del proceso. */
    uint32_t start_offset;        /** @brief Offset de inicio del bloque en el archivo. */
    uint32_t num_pages;           /** @brief Número de páginas que ocupa el proceso. */
    bool is_used;                 /** @brief Si el bloque está ocupado o libre. */
} t_swap_process_info;

#endif