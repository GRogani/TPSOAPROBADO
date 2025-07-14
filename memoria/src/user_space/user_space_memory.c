#include "user_space_memory.h"
#include "../semaphores.h"

static char* user_space = NULL;
static size_t user_space_size = 0;

pthread_mutex_t user_space_mutex;

void init_user_space(size_t size) {
    pthread_mutex_init(&user_space_mutex, NULL);
    
    user_space = malloc(size);
    if (user_space == NULL) {
        LOG_ERROR("Failed to allocate user space memory of size %zu", size);
        exit(EXIT_FAILURE);
    }
    
    memset(user_space, 0, size);
    user_space_size = size;
    
    LOG_INFO("User space memory initialized with size %zu bytes", size);
}

void destroy_user_space(void) {
    if (user_space != NULL) {
        free(user_space);
        user_space = NULL;
        user_space_size = 0;
        
        pthread_mutex_destroy(&user_space_mutex);
        LOG_INFO("User space memory destroyed");
    }
}

void write_to_user_space(uint32_t physical_address, void* data, uint32_t size) {
    if (physical_address + size > user_space_size) {
        LOG_ERROR("Memory access violation: Attempted write to address %u with size %u exceeds user space size %zu",
                 physical_address, size, user_space_size);
        return;
    }
    
    pthread_mutex_lock(&user_space_mutex);
    
    memcpy(&user_space[physical_address], data, size);
    
    pthread_mutex_unlock(&user_space_mutex);
    
    LOG_DEBUG("Written %u bytes to physical address %u", size, physical_address);
}

void read_from_user_space(uint32_t physical_address, void* buffer, uint32_t size) {
    if (physical_address + size > user_space_size) {
        LOG_ERROR("Memory access violation: Attempted read from address %u with size %u exceeds user space size %zu",
                 physical_address, size, user_space_size);
        memset(buffer, 0, size);
        return;
    }
    
    pthread_mutex_lock(&user_space_mutex);
    
    memcpy(buffer, &user_space[physical_address], size);
    
    pthread_mutex_unlock(&user_space_mutex);
    
    LOG_DEBUG("Read %u bytes from physical address %u", size, physical_address);
}

size_t get_user_space_size(void) {
    return user_space_size;
}
