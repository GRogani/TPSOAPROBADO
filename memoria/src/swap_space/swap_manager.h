#ifndef SWAP_SPACE_SWAP_MANAGER_H
#define SWAP_SPACE_SWAP_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "../utils.h"
#include "../kernel_space/process_manager.h"
#include "../user_space/user_space_memory.h"
#include "../user_space/frame_manager.h"

void swap_manager_init(t_memoria_config memoria_config);

void swap_manager_destroy();

/**
 * @brief Allocate frames in the swap space.
 */
t_list *swap_allocate_frames(int32_t pages_needed);

/**
 * @brief Write data to a specific frame in swap space.
 */
bool swap_write_frame(int32_t frame_num, void *data, int32_t size);

/**
 * @brief Read data from a specific frame in swap space.
 */
bool swap_read_frame(int32_t frame_num, void *buffer, int32_t size);

/**
 * @brief Release allocated frames in swap space.
 */
void swap_release_frames(t_list *frame_list);

#endif
