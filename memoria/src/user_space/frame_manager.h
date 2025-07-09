
#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include <math.h>
#include  <commons/collections/list.h>
#include "../utils.h"
#include "semaphores.h"

/**
 * @biref Inicializa el espacio de memoria de usuario, segun el archivo config.
 * @param config Configuracion de memoria que contiene el tamanio de memoria y tamanio de pagina.
 * @return true si la memoria se inicializo correctamente, false en caso contrario.
 */
bool init_user_memory(const t_memoria_config* config);

/**
 * @brief Inicializa el Frame Manager, creando un bitmap de frames libres.
 * Debe ser llamado después de init_user_memory.
 * @note No usa el bitarray de commons, sino un array de bools.
 */
void frame_allocation_init();

/**
 * @brief Reserva un numero de frames
 * @param num_frames Cantidad de frames a reservar
 * @return Lista de numeros de frames asignados, o NULL si no hay suficientes frames libres
 * @note Esta funcion es usada por process manager, ahi se crea la tabla de paginas
 */
t_list *frame_allocate_frames(uint32_t num_frames); // Allocates frames and returns their numbers

void frame_free_frames(t_list* frame_numbers_list);

uint32_t frame_get_free_count(); // Gets number of free frames

// Direct physical memory access operations
bool read_memory(uint32_t physical_address, void* buffer, size_t size);
bool read_full_page(uint32_t physical_address, void* buffer);
bool write_memory(uint32_t physical_address, const void* data, size_t size);
bool update_full_page(uint32_t physical_address, const void* data);

void memory_manager_destroy();

#endif