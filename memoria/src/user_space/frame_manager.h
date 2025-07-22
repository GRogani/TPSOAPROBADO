
#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include <stdatomic.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include "../utils.h"

int frame_get_free_count();

t_list *allocate_frames(int32_t pages_needed);

void frame_allocation_init(t_memoria_config memoria_config);

void release_frames(t_list *frame_list);

#endif