#ifndef UTILS_MMU_REQUEST_PAGE_READ_RESPONSE_H
#define UTILS_MMU_REQUEST_PAGE_READ_RESPONSE_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct t_mmu_page_read_response {
    void* page_data;
    int32_t page_size;
} t_mmu_page_read_response;

t_mmu_page_read_response* read_mmu_page_read_response(t_package* package);

t_package* create_mmu_page_read_response(void* page_data, int32_t page_size);

int send_mmu_page_read_response(int socket, void* page_data, int32_t page_size);

void destroy_mmu_page_read_response(t_mmu_page_read_response* response);

#endif