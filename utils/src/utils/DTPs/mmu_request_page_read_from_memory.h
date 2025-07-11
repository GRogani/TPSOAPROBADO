#ifndef UTILS_MMU_REQUEST_PAGE_READ_FROM_MEMORY_H
#define UTILS_MMU_REQUEST_PAGE_READ_FROM_MEMORY_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct t_mmu_page_read_request {
    uint32_t frame_number;
} t_mmu_page_read_request;

t_mmu_page_read_request* read_mmu_page_read_request(t_package* package);

t_package* create_mmu_page_read_request(uint32_t frame_number);

int send_mmu_page_read_request(int socket, uint32_t frame_number);

void destroy_mmu_page_read_request(t_mmu_page_read_request* request);

#endif