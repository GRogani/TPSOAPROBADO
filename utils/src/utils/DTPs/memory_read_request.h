#ifndef DTO_MEMORY_READ_REQUEST_H
#define DTO_MEMORY_READ_REQUEST_H

#include <stdint.h>
#include <stddef.h>
#include "../serialization/buffer.h"
#include "../serialization/package.h"
#include "utils/safe_alloc.h"


typedef struct {
    int32_t physical_address;
    int32_t size;
} t_memory_read_request;

t_memory_read_request* create_memory_read_request(int32_t physical_address, int32_t size);
void destroy_memory_read_request(t_memory_read_request* request);
void send_memory_read_request(int socket, t_memory_read_request* request);
t_memory_read_request* read_memory_read_request(t_package* package);

#endif