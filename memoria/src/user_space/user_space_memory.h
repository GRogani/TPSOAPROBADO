#ifndef USER_SPACE_MEMORY_H
#define USER_SPACE_MEMORY_H

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include "../utils.h"
#include "../semaphores.h"

void init_user_space(size_t size);
void destroy_user_space(void);
void write_to_user_space(uint32_t physical_address, void* data, uint32_t size);
void read_from_user_space(uint32_t physical_address, void* buffer, uint32_t size);

size_t get_user_space_size(void);

#endif