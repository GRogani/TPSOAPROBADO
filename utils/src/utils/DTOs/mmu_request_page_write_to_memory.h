#ifndef UTILS_MMU_REQUEST_PAGE_WRITE_TO_MEMORY_H
#define UTILS_MMU_REQUEST_PAGE_WRITE_TO_MEMORY_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct t_mmu_page_write_request {
    uint32_t frame_number;
    uint32_t content_size;
    void* content;
} t_mmu_page_write_request;

t_mmu_page_write_request* read_mmu_page_write_request(t_package* package);

t_package* create_mmu_page_write_request(uint32_t frame_number, void* content, uint32_t content_size);

int send_mmu_page_write_request(int socket, uint32_t frame_number, void* content, uint32_t content_size);

void destroy_mmu_page_write_request(t_mmu_page_write_request* request);

#endif