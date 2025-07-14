
#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include "../utils.h"
#include <stdatomic.h>
#include <commons/bitarray.h>
#include <pthread.h>

extern t_memoria_config memoria_config;

int frame_get_free_count();

t_list *allocate_frames(uint32_t pages_needed);

void frame_allocation_init();

void release_frames(t_list *frame_list);

#endif