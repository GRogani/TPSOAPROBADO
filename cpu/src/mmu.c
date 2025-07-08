#include "mmu.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils/safe_alloc.h"


TLBConfig* g_tlb_config;
CacheConfig* g_cache_config;
MMUConfig* g_mmu_config;

static t_list* g_tlb;
static t_list* g_cache;

static int g_tlb_fifo_pointer = 0;
static uint64_t g_lru_timestamp_counter = 0;
static int g_cache_clock_pointer = 0;


uint32_t mmu_request_pagetable_entry_from_memory(uint32_t table_id, uint32_t entry_index) { 
    //TODO: Pegarle a la memoria pasando el entry_index y el table_id, el table_id debe ser un decimal, esta funcion pega en memoria y devuelve un table_id o un frame_number
    LOG_DEBUG("[MEM-REQUEST] Asking Memory for entry %u from table %u", entry_index, table_id);

    if (table_id == 0) { 
        return entry_index * g_mmu_config->entries_per_table;
    } else {
        return table_id + entry_index;
    }
}


void mmu_request_page_read_from_memory(int* memory_socket, uint32_t frame_number, void* buffer) {
    LOG_DEBUG("[MEM-REQUEST] Asking Memory to read page from frame %u", frame_number);
    send_mmu_page_read_request(*memory_socket, frame_number);
    
    // Wait for response from memory
    t_package* response_package = recv_package(*memory_socket);
    if (response_package == NULL || response_package->opcode != READ_MEMORY) {
        LOG_ERROR("[MEM-REQUEST] Failed to receive valid response for page read from frame %u", frame_number);
        if (response_package) package_destroy(response_package);
        return;
    }
    
    // Parse the response and copy data to buffer
    t_mmu_page_read_response* response = read_mmu_page_read_response(response_package);
    package_destroy(response_package);
    
    if (response && response->page_data) {
        memcpy(buffer, response->page_data, response->page_size);
        LOG_DEBUG("[MEM-REQUEST] Successfully read %u bytes from frame %u", response->page_size, frame_number);
    } else {
        LOG_ERROR("[MEM-REQUEST] Invalid response data for page read from frame %u", frame_number);
    }
    
    destroy_mmu_page_read_response(response);
}


void mmu_request_page_write_to_memory(int* memory_socket, uint32_t frame_number, void* content) {
    LOG_INFO("[MEM-REQUEST] Telling Memory to write page to frame %u", frame_number);
    uint32_t content_size = g_mmu_config->page_size;
    send_mmu_page_write_request(*memory_socket, frame_number, content, content_size);
}


void tlb_entry_destroy(void* element) {
    free(element);
}

void cache_entry_destroy(void* element) {
    CacheEntry* entry = (CacheEntry*)element;
    free(entry->content);
    free(entry);
}


TLBEntry* tlb_find_entry(uint32_t page_number) {
    if (g_tlb_config->entry_count == 0) return NULL;

    bool _is_page(void* element) {
        TLBEntry* entry = (TLBEntry*)element;
        return entry->page == page_number;
    }

    TLBEntry* entry = list_find(g_tlb, _is_page);

    if (entry != NULL && g_tlb_config->replacement_algo == TLB_ALGO_LRU) {
        entry->lru_timestamp = g_lru_timestamp_counter++;
    }
    return entry;
}

void tlb_add_entry(uint32_t page_number, uint32_t frame_number) {
    if (g_tlb_config->entry_count == 0) return;

    TLBEntry* new_entry = malloc(sizeof(TLBEntry));
    new_entry->page = page_number;
    new_entry->frame = frame_number;
    new_entry->lru_timestamp = g_lru_timestamp_counter++;

    if (list_size(g_tlb) < g_tlb_config->entry_count) {
        list_add(g_tlb, new_entry);
        LOG_DEBUG("[TLB] Added page %u -> frame %u to a free slot.", page_number, frame_number);
    } else {
        int victim_index = -1;
        if (g_tlb_config->replacement_algo == TLB_ALGO_FIFO) {
            victim_index = g_tlb_fifo_pointer;
            g_tlb_fifo_pointer = (g_tlb_fifo_pointer + 1) % g_tlb_config->entry_count;
        } else { 
            uint64_t min_ts = UINT64_MAX;
            for (int i = 0; i < list_size(g_tlb); i++) {
                TLBEntry* entry = list_get(g_tlb, i);
                if (entry->lru_timestamp < min_ts) {
                    min_ts = entry->lru_timestamp;
                    victim_index = i;
                }
            }
        }
        TLBEntry* victim = list_get(g_tlb, victim_index);
        LOG_DEBUG("[TLB] Replacing victim (page %u) at index %d with new entry (page %u).", victim->page, victim_index, page_number);
        list_replace_and_destroy_element(g_tlb, victim_index, new_entry, tlb_entry_destroy);
    }
}


uint32_t mmu_perform_page_walk(uint32_t page_number) { 
    LOG_INFO("[MMU] Page Walk starting for page number %u...", page_number);

    uint32_t next_table_id = 0; // Level 1 table ID is 0 by convention

    for (int level = 1; level <= g_mmu_config->page_table_levels; level++) {

        int power = g_mmu_config->page_table_levels - level;
        uint32_t divisor = (uint32_t)pow(g_mmu_config->entries_per_table, power);
        uint32_t entry_index = (page_number / divisor) % g_mmu_config->entries_per_table;
        
        LOG_DEBUG("  - Level %d: Power %d, Divisor %u -> table entry %u", 
                  level, power, divisor, entry_index);
        
        next_table_id = mmu_request_pagetable_entry_from_memory(next_table_id, entry_index);
        
        LOG_DEBUG("    Next table/frame ID: %u", next_table_id);
    }
    
    uint32_t frame_number = next_table_id;
    LOG_INFO("[MMU] Page Walk complete. Page number %u maps to Frame %u.", page_number, frame_number);
    return frame_number;
}

uint32_t mmu_translate_address(uint32_t logical_address) {
    
    uint32_t page_number = logical_address / g_mmu_config->page_size;  // floor() is implicit with integer division
    uint32_t offset = logical_address % g_mmu_config->page_size;
    
    LOG_INFO("--- Translating Logical Address %u ---", logical_address);
    LOG_DEBUG("  -> Page Number: %u, Offset: %u", page_number, offset);

    uint32_t frame_number;
    TLBEntry* tlb_entry = tlb_find_entry(page_number);

    if (tlb_entry) {
        LOG_INFO("[TLB] HIT! Page number %u -> Frame %u.", page_number, tlb_entry->frame);
        frame_number = tlb_entry->frame;
    } else {
        LOG_INFO("[TLB] MISS for page number %u. Consulting page tables...", page_number);
        frame_number = mmu_perform_page_walk(page_number);
        tlb_add_entry(page_number, frame_number);
    }

    uint32_t physical_address = (frame_number * g_mmu_config->page_size) + offset;
    LOG_DEBUG("  -> Resulting Physical Address: %u", physical_address);
    
    return physical_address;
}

/*MMU Administrativos*/
void mmu_init(MMUConfig* mmu_config, TLBConfig* tlb_config, CacheConfig* cache_config) {
    g_mmu_config = mmu_config;
    g_tlb_config = tlb_config;
    g_cache_config = cache_config;

    if (g_tlb_config->entry_count > 0) {
        g_tlb = list_create();
    }
    if (g_cache_config->entry_count > 0) {
        g_cache = list_create();
        for (int i = 0; i < g_cache_config->entry_count; i++) {
            CacheEntry* entry = malloc(sizeof(CacheEntry));
            entry->is_valid = false;
            entry->content = malloc(g_mmu_config->page_size);
            list_add(g_cache, entry);
        }
    }
    LOG_INFO("MMU, TLB, and Cache initialized.");
}


void mmu_process_cleanup(int* memory_socket) {
    LOG_INFO("--- Cleaning up for process eviction ---");
    if (g_cache_config->entry_count > 0) {
        LOG_INFO("[Cache] Flushing dirty pages to memory...");
        void _writeback_if_dirty(void* element) {
            CacheEntry* entry = (CacheEntry*)element;
            if (entry->is_valid && entry->modified_bit) {
                
                mmu_request_page_write_to_memory(memory_socket,entry->frame, entry->content);
            }
            entry->is_valid = false; // Invalidate all entries
        }
        list_iterate(g_cache, _writeback_if_dirty);
        LOG_INFO("[Cache] All cache entries invalidated.");
    }
    if (g_tlb_config->entry_count > 0) {
        list_clean_and_destroy_elements(g_tlb, tlb_entry_destroy);
        LOG_INFO("[TLB] All TLB entries flushed.");
    }
}

void mmu_destroy() {
    LOG_INFO("Destroying MMU resources...");
    if (g_tlb) {
        list_destroy_and_destroy_elements(g_tlb, tlb_entry_destroy);
    }
    if (g_cache) {
        list_destroy_and_destroy_elements(g_cache, cache_entry_destroy);
    }
}
/*MMU Administrativos*/



/*CACHE*/


CacheEntry* cache_find_entry(uint32_t page_number) {
    if (g_cache_config->entry_count == 0) return NULL;

    bool _is_frame(void* element) {
        CacheEntry* entry = (CacheEntry*)element;
        return entry->is_valid && entry->frame == page_number;
    }

    CacheEntry* entry = list_find(g_cache, _is_frame);
    if (entry) {
        entry->use_bit = true;
    }
    return entry;
}

int cache_find_victim_clock() {
    while (true) {
        CacheEntry* entry = list_get(g_cache, g_cache_clock_pointer);
        if (!entry->is_valid) {
            return g_cache_clock_pointer; // Found an unused slot
        }
        if (entry->use_bit) {
            entry->use_bit = false; // Give a second chance
        } else {
            // Found a victim (use_bit == 0)
            int victim_index = g_cache_clock_pointer;
            g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
            return victim_index;
        }
        g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
    }
}

int cache_find_victim_clock_m() {
    for (int i = 0; i < g_cache_config->entry_count * 2; i++) {
        CacheEntry* entry = list_get(g_cache, g_cache_clock_pointer);
        if (!entry->is_valid) return g_cache_clock_pointer;
        if (!entry->use_bit && !entry->modified_bit) {
            int victim_index = g_cache_clock_pointer;
            g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
            return victim_index;
        }
        g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
    }
    for (int i = 0; i < g_cache_config->entry_count * 2; i++) {
        CacheEntry* entry = list_get(g_cache, g_cache_clock_pointer);
        if (!entry->use_bit && entry->modified_bit) {
            int victim_index = g_cache_clock_pointer;
            g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
            return victim_index;
        }
        entry->use_bit = false;
        g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
    }
    return cache_find_victim_clock();
}

CacheEntry* cache_load_page(uint32_t page_number, int* memory_socket) {
    LOG_INFO("[Cache] MISS for frame %u. Finding a victim to replace...", page_number);
    
    int victim_index;
    if (g_cache_config->replacement_algo == CACHE_ALGO_CLOCK) {
        victim_index = cache_find_victim_clock();
    } else { 
        victim_index = cache_find_victim_clock_m();
    }

    CacheEntry* victim_entry = list_get(g_cache, victim_index);
    if (victim_entry->is_valid && victim_entry->modified_bit) {
        LOG_INFO("[Cache] Victim (frame %u) is dirty. Writing back to memory.", victim_entry->frame);
        mmu_request_page_write_to_memory(memory_socket,victim_entry->frame, victim_entry->content);
    }
    
    LOG_DEBUG("[Cache] Loading frame %u into cache slot %d.", page_number, victim_index);

    //TODO: Pasar por tlb previamente
    mmu_request_page_read_from_memory(memory_socket, page_number, victim_entry->content);

    victim_entry->is_valid = true;
    victim_entry->frame = page_number;
    victim_entry->use_bit = true;
    victim_entry->modified_bit = false;
    
    return victim_entry;
}
/*CACHE*/