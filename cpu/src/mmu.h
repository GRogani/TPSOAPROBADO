#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>

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

typedef enum {
    TLB_ALGO_FIFO,
    TLB_ALGO_LRU
} TLBReplacementAlgorithm;

typedef enum {
    CACHE_ALGO_CLOCK,
    CACHE_ALGO_CLOCK_M
} CacheReplacementAlgorithm;

typedef struct {
    uint32_t page_size;
    int page_table_levels;
    int entries_per_table;
} MMUConfig;

typedef struct {
    int entry_count;
    TLBReplacementAlgorithm replacement_algo;
} TLBConfig;

typedef struct {
    int entry_count;
    CacheReplacementAlgorithm replacement_algo;
} CacheConfig;

void mmu_init(MMUConfig* mmu_config, TLBConfig* tlb_config, CacheConfig* cache_config);

uint32_t mmu_translate_address(uint32_t logical_address);


void mmu_read_byte(uint32_t logical_address);

void mmu_write_byte(uint32_t logical_address);

void mmu_process_cleanup();

void mmu_destroy();

#endif 