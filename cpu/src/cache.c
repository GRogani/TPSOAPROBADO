#include "cache.h"

static MMUConfig* g_mmu_config;

static t_list* g_cache;

static int g_cache_clock_pointer = 0;


static void mmu_request_page_read_from_memory(int* memory_socket, uint32_t frame_number, void* buffer) {
    LOG_DEBUG("[MEM-REQUEST] Asking Memory to read page from frame %u", frame_number);
    send_mmu_page_read_request(memory_socket, frame_number);
    //TODO: Manejar respuesta y guardar en buffer
}


static void mmu_request_page_write_to_memory(int* memory_socket, uint32_t frame_number, void* content) {
    LOG_INFO("[MEM-REQUEST] Telling Memory to write page to frame %u", frame_number);
    uint32_t content_size = g_mmu_config->page_size;
    send_mmu_page_write_request(memory_socket, frame_number, content, content_size);
}

static CacheEntry* cache_find_entry(uint32_t frame_number) {
    if (g_cache_config->entry_count == 0) return NULL;

    bool _is_frame(void* element) {
        CacheEntry* entry = (CacheEntry*)element;
        return entry->is_valid && entry->frame == frame_number;
    }

    CacheEntry* entry = list_find(g_cache, _is_frame);
    if (entry) {
        entry->use_bit = true;
    }
    return entry;
}

static int cache_find_victim_clock() {
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

static int cache_find_victim_clock_m() {
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

static CacheEntry* cache_load_page(uint32_t frame_number, int* memory_socket) {
    LOG_INFO("[Cache] MISS for frame %u. Finding a victim to replace...", frame_number);
    
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
    
    LOG_DEBUG("[Cache] Loading frame %u into cache slot %d.", frame_number, victim_index);
    mmu_request_page_read_from_memory(memory_socket,frame_number, victim_entry->content);

    victim_entry->is_valid = true;
    victim_entry->frame = frame_number;
    victim_entry->use_bit = true;
    victim_entry->modified_bit = false;
    
    return victim_entry;
}
