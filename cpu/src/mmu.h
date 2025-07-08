#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <config_loader.h>
#include "utils/DTOs/mmu_request_page_write_to_memory.h"
#include "utils/DTOs/mmu_request_page_read_from_memory.h"
#include "utils/DTOs/mmu_request_page_read_response.h"

// Global variables declarations
extern TLBConfig* g_tlb_config;
extern CacheConfig* g_cache_config;
extern MMUConfig* g_mmu_config;

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

// MMU Administrativos
void mmu_init(MMUConfig* mmu_config, TLBConfig* tlb_config, CacheConfig* cache_config);
uint32_t mmu_translate_address(int* memory_socket, uint32_t logical_address);
void mmu_process_cleanup(int* memory_socket);
void mmu_destroy();

// Memory Request Functions
uint32_t mmu_request_pagetable_entry_from_memory(int* memory_socket, uint32_t table_id, uint32_t entry_index);
void mmu_request_page_read_from_memory(int* memory_socket, uint32_t frame_number, void* buffer);
void mmu_request_page_write_to_memory(int* memory_socket, uint32_t frame_number, void* content);

// TLB Functions
TLBEntry* tlb_find_entry(uint32_t page_number);
void tlb_add_entry(uint32_t page_number, uint32_t frame_number);

// MMU Page Walk
uint32_t mmu_perform_page_walk(int* memory_socket, uint32_t page_number);

// Cache Functions
CacheEntry* cache_find_entry(uint32_t frame_number);
int cache_find_victim_clock(void);
int cache_find_victim_clock_m(void);
CacheEntry* cache_load_page(uint32_t frame_number, int* memory_socket);

// Utility Functions
void tlb_entry_destroy(void* element);
void cache_entry_destroy(void* element);

#endif