#ifndef DTO_MEMORY_READ_RESPONSE_H
#define DTO_MEMORY_READ_RESPONSE_H

#include <stdint.h>
#include <stddef.h>
#include "../serialization/buffer.h"
#include "../serialization/package.h"

typedef struct {
    char* data;
    int32_t data_size;
} t_memory_read_response;

t_package* create_package_memory_read_response(char* data, int32_t size);
void send_memory_read_response(int socket, char* data, int32_t size);
t_memory_read_response* read_memory_read_response(t_package* package);
void destroy_memory_read_response(t_memory_read_response* response);

#endif