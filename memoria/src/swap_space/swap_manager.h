#ifndef SWAP_SPACE_SWAP_MANAGER_H
#define SWAP_SPACE_SWAP_MANAGER_H

#include "../utils.h"
#include "semaphores.h"
#include "swap_structures.h"


/**
 * @brief Inicializa el sistema de gestión de SWAP, creando el archivo de swap y las estructuras de control.
 * @param config Puntero a la configuración global de la memoria.
 * @return true si la inicialización fue exitosa, false en caso contrario.
 */
bool swap_manager_init(const t_memoria_config* config);

/**
 * @brief Destruye el sistema de gestión de SWAP, cerrando el archivo y liberando todos los recursos.
 */
void swap_manager_destroy(void);

/**
 * @brief Escribe el contenido de una página en el archivo de swap.
 * @param pid PID del proceso al que pertenece la página.
 * @param virtual_page_number Número de página virtual del proceso.
 * @param page_content Puntero al buffer con el contenido de la página a escribir.
 * @return Un puntero a una estructura t_swap_page_info con la ubicación en SWAP, o NULL si falla la escritura/asignación.
 */
t_swap_page_info* swap_manager_write_page_to_swap(uint32_t pid, uint32_t virtual_page_number, const void* page_content);

/**
 * @brief Lee el contenido de una página desde el archivo de swap.
 * @param swap_info Puntero a la estructura t_swap_page_info que contiene la ubicación de la página en SWAP.
 * @param buffer Puntero al buffer donde se copiará el contenido de la página leída.
 * @return true si la lectura fue exitosa, false en caso contrario.
 */
bool swap_manager_read_page_from_swap(const t_swap_page_info* swap_info, void* buffer);

/**
 * @brief Libera el espacio ocupado por una página en el archivo de swap.
 * Esta función es adecuada para ser usada como callback en list_destroy_and_destroy_elements.
 * @param element Puntero a la estructura t_swap_page_info* de la página a liberar.
 */
void swap_manager_free_page_from_swap(void* element);

/**
 * @brief Obtiene la cantidad actual de "páginas" libres (espacios disponibles del tamaño de una página) en el archivo de swap.
 * @return La cantidad de espacios de página libres.
 */
size_t swap_manager_get_free_pages_count(void);

/**
 * @brief Asigna páginas en el archivo de swap para un proceso
 * @param pid ID del proceso
 * @param num_pages_to_swap Número de páginas a asignar
 * @return Lista con información de páginas asignadas o NULL si error
 */
t_list* swap_allocate_pages(uint32_t pid, size_t num_pages_to_swap);

/**
 * @brief Escribe una página en el archivo de swap
 * @param swap_offset Offset en el archivo de swap
 * @param page_content Contenido de la página
 * @param page_size Tamaño de la página
 * @return true si éxito, false si error
 */
bool swap_write_page(uint32_t swap_offset, const void* page_content, size_t page_size);

/**
 * @brief Lee una página desde el archivo de swap
 * @param swap_offset Offset en el archivo de swap
 * @param buffer Buffer donde almacenar la página
 * @param page_size Tamaño de la página
 * @return true si éxito, false si error
 */
bool swap_read_page(uint32_t swap_offset, void* buffer, size_t page_size);

/**
 * @brief Libera páginas en el archivo de swap
 * @param swap_page_info_list Lista con información de páginas a liberar
 * @param page_size Tamaño de cada página
 */
void swap_free_pages(t_list* swap_page_info_list, size_t page_size);

#endif // SWAP_SPACE_SWAP_MANAGER_H