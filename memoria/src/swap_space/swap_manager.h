#ifndef SWAP_SPACE_SWAP_MANAGER_H
#define SWAP_SPACE_SWAP_MANAGER_H

#include "../utils.h"
#include "../semaphores.h"
#include "swap_structures.h"

/**
 * @brief Inicializa el sistema de gestión de SWAP simplificado.
 * @param config Puntero a la configuración global de la memoria.
 * @return true si la inicialización fue exitosa, false en caso contrario.
 */
bool swap_manager_init(const t_memoria_config* config);

/**
 * @brief Destruye el sistema de gestión de SWAP, cerrando el archivo y liberando todos los recursos.
 */
void swap_manager_destroy(void);

/**
 * @brief Asigna páginas en el archivo de swap para un proceso completo.
 * Busca espacio libre al inicio del archivo, si no encuentra va al final.
 * @param pid ID del proceso
 * @param num_pages Número de páginas a asignar
 * @return Lista con información de páginas asignadas o NULL si error
 */
t_list* swap_allocate_pages(uint32_t pid, uint32_t num_pages);

/**
 * @brief Escribe todas las páginas de un proceso al archivo de swap.
 * @param pages Lista con información de páginas del proceso
 * @param process_memory Puntero a la memoria del proceso
 * @param page_size Tamaño de cada página
 * @return true si éxito, false si error
 */
bool swap_write_pages(t_list* pages, void* process_memory, uint32_t page_size);

/**
 * @brief Lee todas las páginas de un proceso desde el archivo de swap.
 * @param pages Lista con información de páginas del proceso
 * @param process_memory Puntero a la memoria del proceso
 * @param page_size Tamaño de cada página
 * @return true si éxito, false si error
 */
bool swap_read_pages(t_list* pages, void* process_memory, uint32_t page_size);

/**
 * @brief Libera las páginas de un proceso en el archivo de swap (marca como libre).
 * @param pid ID del proceso a liberar
 */
void swap_free_pages(uint32_t pid);

/**
 * @brief Obtiene la cantidad actual de páginas libres en el archivo de swap.
 * @return La cantidad de páginas libres.
 */
size_t swap_get_free_pages_count(void);

#endif // SWAP_SPACE_SWAP_MANAGER_H