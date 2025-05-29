#ifndef UTILS_MEMORY_FREE_SPACE_H
#define UTILS_MEMORY_FREE_SPACE_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct t_memory_free_space {
    uint32_t free_space;
} t_memory_free_space;

t_memory_free_space* read_memory_free_space(t_package* package);

t_package* create_memory_free_space_request(void);

int send_memory_free_space_request(int socket);

void destroy_memory_free_space(t_memory_free_space* response);

#endif
