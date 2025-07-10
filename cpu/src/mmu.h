#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <config_loader.h>
#include "utils/DTPs/mmu_request_page_write_to_memory.h"
#include "utils/DTPs/mmu_request_page_read_from_memory.h"
#include "utils/DTPs/mmu_request_page_read_response.h"
#include "utils/DTPs/memory_read_request.h"
#include "utils/DTPs/memory_read_response.h"

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
    uint32_t page;
    void* content;
    bool use_bit;
    bool modified_bit;
 } CacheEntry;

// MMU Administrativos
void mmu_init(MMUConfig* mmu_config, TLBConfig* tlb_config, CacheConfig* cache_config);
uint32_t mmu_translate_address(int* memory_socket, uint32_t logical_address,uint32_t pid);
void mmu_process_cleanup(int* memory_socket);
void mmu_destroy();

// Memory Request Functions
uint32_t mmu_request_pagetable_entry_from_memory(int* memory_socket, uint32_t table_id, uint32_t entry_index,uint32_t pid);
void mmu_request_page_read_from_memory(int* memory_socket, uint32_t physic_addr, void* buffer);
void mmu_request_page_write_to_memory(int* memory_socket, uint32_t physic_addr, void* content);

// TLB Functions
TLBEntry* tlb_find_entry(uint32_t page_number);
void tlb_add_entry(uint32_t page_number, uint32_t frame_number);

// MMU Page Walk
uint32_t mmu_perform_page_walk(int* memory_socket, uint32_t page_number, uint32_t pid);

// Cache Functions
CacheEntry* cache_find_entry(uint32_t page_number);
int cache_find_victim_clock(void);
int cache_find_victim_clock_m(void);
CacheEntry* cache_load_page(uint32_t logic_dir, int* memory_socket, CacheEntry *victim_entry, uint32_t pid);
CacheEntry* select_victim_entry(int *memory_socket, uint32_t logic_dir, uint32_t pid);

// Utility Functions
void tlb_entry_destroy(void* element);
void cache_entry_destroy(void* element);

#endif