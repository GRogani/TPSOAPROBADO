#ifndef SWAP_SPACE_SWAP_MANAGER_H
#define SWAP_SPACE_SWAP_MANAGER_H

#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "../kernel_space/process_manager.h"
#include "../user_space/user_space_memory.h"
#include "../user_space/frame_manager.h"

void swap_manager_init();

void swap_manager_destroy();

/**
 * @brief Allocate frames in the swap space.
 */
t_list *swap_allocate_frames(uint32_t pages_needed);

/**
 * @brief Write data to a specific frame in swap space.
 */
int swap_write_frame(uint32_t frame_num, void *data, uint32_t size);

/**
 * @brief Read data from a specific frame in swap space.
 */
int swap_read_frame(uint32_t frame_num, void *buffer, uint32_t size);

/**
 * @brief Release allocated frames in swap space.
 */
void swap_release_frames(t_list *frame_list);

#endif
