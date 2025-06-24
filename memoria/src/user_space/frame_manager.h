
#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include <math.h>
#include  <commons/collections/list.h>
#include "../utils.h"
#include "semaphores.h"

// Initialization and destruction of user memory space
bool init_user_memory(const t_memoria_config* config);
void memory_manager_destroy();

// Direct physical memory access operations
bool read_memory(uint32_t physical_address, void* buffer, size_t size);
bool read_full_page(uint32_t physical_address, void* buffer);
bool write_memory(uint32_t physical_address, const void* data, size_t size);
bool update_full_page(uint32_t physical_address, const void* data);

// Frame allocation management
void frame_allocation_init(); // Initializes the internal bitmap for frames
t_list* frame_allocate_frames(size_t num_frames); // Allocates frames and returns their numbers
void frame_free_frames(t_list* frame_numbers_list); // Frees frames
size_t frame_get_free_count(); // Gets number of free frames

#endif