#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <config_loader.h>
#include "utils/DTPs/mmu_request_page_read_response.h"
#include "utils/DTPs/memory_read_request.h"
#include "utils/DTPs/memory_read_response.h"

// Global variables declarations
extern TLBConfig* g_tlb_config;
extern CacheConfig* g_cache_config;
extern MMUConfig* g_mmu_config;

typedef struct {
    int32_t page;
    int32_t pid;
    int32_t frame;
    uint64_t lru_timestamp; 
} TLBEntry;

typedef struct {
    bool is_valid;
    int32_t page;
    int32_t pid;
    void* content;
    bool use_bit;
    bool modified_bit;
    int32_t modified_start; // Start offset of modified region
    int32_t modified_end;   // End offset of modified region (exclusive)
 } CacheEntry;

// MMU Administrativos
void mmu_init(MMUConfig* mmu_config, TLBConfig* tlb_config, CacheConfig* cache_config);
int32_t mmu_translate_address(int memory_socket, int32_t logical_address, int32_t pid);
void mmu_process_cleanup(int memory_socket);
void mmu_destroy();

// Memory Request Functions
int32_t mmu_request_pagetable_entry_from_memory(int memory_socket, int32_t table_id, int32_t entry_index, int32_t pid);
void mmu_request_page_read_from_memory(int memory_socket, int32_t physic_addr, void* buffer);
void mmu_request_page_write_to_memory(int memory_socket, int32_t physic_addr, void* content);

// TLB Functions
TLBEntry* tlb_find_entry(int32_t page_number, int32_t pid);
void tlb_add_entry(int32_t page_number, int32_t frame_number, int32_t pid);

// MMU Page Walk
int32_t mmu_perform_page_walk(int memory_socket, int32_t page_number, int32_t pid);

// Cache Functions
CacheEntry *cache_find_entry(int32_t page_number, int32_t pid);
int cache_find_victim_clock(void);
int cache_find_victim_clock_m(void);
CacheEntry* cache_load_page(int32_t logic_dir, int memory_socket, CacheEntry *victim_entry, int32_t pid);
CacheEntry* select_victim_entry(int memory_socket, int32_t pid);

// Utility Functions
void tlb_entry_destroy(void* element);
void cache_entry_destroy(void* element);

#endif
