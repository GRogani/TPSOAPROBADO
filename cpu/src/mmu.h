#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <config_loader.h>
#include "utils/DTOs/mmu_request_page_write_to_memory.h"
#include "utils/DTOs/mmu_request_page_read_from_memory.h"
typedef struct {
    uint32_t page;
    uint32_t frame;
    uint64_t lru_timestamp; 
} TLBEntry;

typedef struct {
    bool is_valid;
    uint32_t frame;
    void* content;
    bool use_bit;
    bool modified_bit;
 } CacheEntry;

void mmu_init(MMUConfig* mmu_config, TLBConfig* tlb_config, CacheConfig* cache_config);

uint32_t mmu_translate_address(uint32_t logical_address);

void mmu_process_cleanup();

void mmu_destroy();

#endif 