#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H
#include <commons/config.h>
#include <stdint.h>
#include "../utils.h"


typedef enum {
    TLB_ALGO_FIFO,
    TLB_ALGO_LRU
} TLBReplacementAlgorithm;

typedef enum {
    CACHE_ALGO_CLOCK,
    CACHE_ALGO_CLOCK_M
} CacheReplacementAlgorithm;

typedef struct {
    int32_t page_size;
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
    int retardo_cache;  // Delay in milliseconds for cache access
} CacheConfig;

MMUConfig load_mmu_config(char *argv[]);

TLBConfig load_tlb_config(char *argv[]);

CacheConfig load_cache_config(t_config* config);

#endif 