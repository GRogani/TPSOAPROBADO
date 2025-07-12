#include "mmu.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils/safe_alloc.h"
#include "utils/serialization/package.h"
#include "utils/DTPs/mmu_request_page_read_response.h"
#include "utils/DTPs/mmu_request_page_write_to_memory.h"
#include "utils/DTPs/memory_read_request.h"
#include "utils/DTPs/memory_read_response.h"

TLBConfig *g_tlb_config;
CacheConfig *g_cache_config;
MMUConfig *g_mmu_config;

static t_list *g_tlb;
static t_list *g_cache;

static int g_tlb_fifo_pointer = 0;
static uint64_t g_lru_timestamp_counter = 0;
static int g_cache_clock_pointer = 0;

uint32_t mmu_request_pagetable_entry_from_memory(int memory_socket, uint32_t table_level, uint32_t entry_index, uint32_t pid)
{
  LOG_DEBUG("[MEM-REQUEST] Requesting page table entry from memory. PID: %u, Table Level: %u, Entry Index: %u", pid, table_level, entry_index);
  send_page_entry_request_package(memory_socket, pid, table_level, entry_index);

  // Wait for response from memory
  t_package *response_package = recv_package(memory_socket);
  if (response_package == NULL || response_package->opcode != GET_PAGE_ENTRY)
  {
    LOG_ERROR("[MEM-REQUEST] Failed to receive valid response for page table entry request");
    if (response_package)
      destroy_package(response_package);
    return -1; // Error
  }

  // Parse the response
  page_entry_response_data response = read_page_entry_response_package(response_package);
  destroy_package(response_package);

  LOG_DEBUG("[MEM-REQUEST] Received value %u from memory", response.value);
  return response.value;
}

void mmu_request_page_read_from_memory(int memory_socket, uint32_t physic_addr, void *buffer)
{
  LOG_DEBUG("[MEM-REQUEST] Asking Memory to read page from physic address %u", physic_addr);

  t_memory_read_request *request = create_memory_read_request(physic_addr, g_mmu_config->page_size);
  send_memory_read_request(memory_socket, request);
  destroy_memory_read_request(request);

  // Wait for response from memory
  t_package *response_package = recv_package(memory_socket);
  if (response_package == NULL || response_package->opcode != READ_MEMORY)
  {
    LOG_ERROR("[MEM-REQUEST] Failed to receive valid response for page read request");
    if (response_package)
      destroy_package(response_package);
    return;
  }

  // Parse the response and copy data to buffer
  t_memory_read_response *response = read_memory_read_response(response_package);
  destroy_package(response_package);

  if (response && response->data)
  {
    memcpy(buffer, response->data, response->data_size);
    LOG_DEBUG("[MEM-REQUEST] Successfully read %u bytes from physic address %u", response->data_size, physic_addr);
  }
  else
  {
    LOG_ERROR("[MEM-REQUEST] Invalid response data for page read from physic address %u", physic_addr);
  }

  destroy_memory_read_response(response);
}

void mmu_request_page_write_to_memory(int memory_socket, uint32_t physic_addr, void *content)
{
  LOG_INFO("[MEM-REQUEST] Telling Memory to write page to physic addres %u", physic_addr);
  uint32_t content_size = g_mmu_config->page_size;
  t_memory_write_request *request = create_memory_write_request(physic_addr, content_size, content);
  send_memory_write_request(memory_socket, request);
  
  // Wait for confirmation
  t_package* response = recv_package(memory_socket);
  if (response != NULL) {
    if (response->opcode == CONFIRMATION) {
      bool success = read_confirmation_package(response);
      if (!success) {
        LOG_ERROR("[MEM-REQUEST] Write to physical address %u failed", physic_addr);
      }
    } else {
      LOG_ERROR("[MEM-REQUEST] Unexpected response opcode: %d", response->opcode);
    }
    destroy_package(response);
  } else {
    LOG_ERROR("[MEM-REQUEST] No response received for write to physical address %u", physic_addr);
  }
  
  destroy_memory_write_request(request);
}

void tlb_entry_destroy(void *element)
{
  free(element);
}

void cache_entry_destroy(void *element)
{
  CacheEntry *entry = (CacheEntry *)element;
  free(entry->content);
  free(entry);
}

TLBEntry *tlb_find_entry(uint32_t page_number)
{
  if (g_tlb_config->entry_count == 0)
    return NULL;

  bool _is_page(void *element)
  {
    TLBEntry *entry = (TLBEntry *)element;
    return entry->page == page_number;
  }

  TLBEntry *entry = list_find(g_tlb, _is_page);

  if (entry != NULL && g_tlb_config->replacement_algo == TLB_ALGO_LRU)
  {
    entry->lru_timestamp = g_lru_timestamp_counter++;
  }
  return entry;
}

void tlb_add_entry(uint32_t page_number, uint32_t frame_number)
{
  if (g_tlb_config->entry_count == 0)
    return;

  TLBEntry *new_entry = malloc(sizeof(TLBEntry));
  new_entry->page = page_number;
  new_entry->frame = frame_number;
  new_entry->lru_timestamp = g_lru_timestamp_counter++;

  if (list_size(g_tlb) < g_tlb_config->entry_count)
  {
    list_add(g_tlb, new_entry);
    LOG_DEBUG("[TLB] Added page %u -> frame %u to a free slot.", page_number, frame_number);
  }
  else
  {
    int victim_index = -1;
    if (g_tlb_config->replacement_algo == TLB_ALGO_FIFO)
    {
      victim_index = g_tlb_fifo_pointer;
      g_tlb_fifo_pointer = (g_tlb_fifo_pointer + 1) % g_tlb_config->entry_count;
    }
    else
    {
      uint64_t min_ts = UINT64_MAX;
      for (int i = 0; i < list_size(g_tlb); i++)
      {
        TLBEntry *entry = list_get(g_tlb, i);
        if (entry->lru_timestamp < min_ts)
        {
          min_ts = entry->lru_timestamp;
          victim_index = i;
        }
      }
    }
    TLBEntry *victim = list_get(g_tlb, victim_index);
    LOG_DEBUG("[TLB] Replacing victim (page %u) at index %d with new entry (page %u).", victim->page, victim_index, page_number);
    list_replace_and_destroy_element(g_tlb, victim_index, new_entry, tlb_entry_destroy);
  }
}

uint32_t mmu_perform_page_walk(int memory_socket, uint32_t page_number, uint32_t pid)
{
  LOG_INFO("[MMU] Page Walk starting for page number %u...", page_number);

  uint32_t next_table_id = 0; // Level 1 table ID is 0 by convention

  for (int level = 1; level <= g_mmu_config->page_table_levels; level++)
  {

    int power = g_mmu_config->page_table_levels - level;
    uint32_t divisor = (uint32_t)pow(g_mmu_config->entries_per_table, power);
    uint32_t entry_index = (page_number / divisor) % g_mmu_config->entries_per_table;

    LOG_DEBUG("  - Level %d: Power %d, Divisor %u -> table entry %u",
              level, power, divisor, entry_index);

    next_table_id = mmu_request_pagetable_entry_from_memory(memory_socket, level, entry_index, pid);

    LOG_DEBUG("    Next table/frame ID: %u", next_table_id);
  }

  uint32_t frame_number = next_table_id;
  LOG_OBLIGATORIO("PID: %d - OBTENER MARCO - Página: %u - Marco: %u", pid, page_number, frame_number);
  return frame_number;
}

uint32_t mmu_translate_address(int memory_socket, uint32_t logical_address, uint32_t pid)
{

  uint32_t page_number = logical_address / g_mmu_config->page_size; // floor() is implicit with integer division
  uint32_t offset = logical_address % g_mmu_config->page_size;

  LOG_INFO("--- Translating Logical Address %u ---", logical_address);
  LOG_DEBUG("  -> Page Number: %u, Offset: %u", page_number, offset);

  uint32_t frame_number;
  TLBEntry *tlb_entry = tlb_find_entry(page_number);

  if (tlb_entry)
  {
    LOG_OBLIGATORIO("PID: %d - TLB HIT - Pagina: %u", pid, page_number);
    frame_number = tlb_entry->frame;
  }
  else
  {
    LOG_OBLIGATORIO("PID: %d - TLB MISS - Pagina: %u", pid, page_number);
    frame_number = mmu_perform_page_walk(memory_socket, page_number, pid);
    tlb_add_entry(page_number, frame_number);
  }

  uint32_t physical_address = (frame_number * g_mmu_config->page_size) + offset;
  LOG_DEBUG("  -> Resulting Physical Address: %u", physical_address);

  return physical_address;
}

/*MMU Administrativos*/
void mmu_init(MMUConfig *mmu_config, TLBConfig *tlb_config, CacheConfig *cache_config)
{
  g_mmu_config = mmu_config;
  g_tlb_config = tlb_config;
  g_cache_config = cache_config;

  if (g_tlb_config->entry_count > 0)
  {
    g_tlb = list_create();
  }
  if (g_cache_config->entry_count > 0)
  {
    g_cache = list_create();
    for (int i = 0; i < g_cache_config->entry_count; i++)
    {
      CacheEntry *entry = malloc(sizeof(CacheEntry));
      entry->is_valid = false;
      entry->content = safe_calloc(1, g_mmu_config->page_size); // Initialize with zeros
      list_add(g_cache, entry);
    }
  }
  LOG_INFO("MMU, TLB, and Cache initialized.");
}

void mmu_process_cleanup(int memory_socket)
{
  LOG_INFO("--- Cleaning up for process eviction ---");
  if (g_cache_config->entry_count > 0)
  {
    LOG_INFO("[Cache] Flushing dirty pages to memory...");
    void _writeback_if_dirty(void *element)
    {
      CacheEntry *entry = (CacheEntry *)element;
      if (entry->is_valid && entry->modified_bit)
      {
        // Calculate physical address for the dirty page
        uint32_t page_number = entry->page;
        uint32_t physic_addr;
        
        if (g_tlb_config->entry_count > 0) {
          // Use TLB to get frame number
          TLBEntry* tlb_entry = tlb_find_entry(page_number);
          if (tlb_entry) {
            physic_addr = tlb_entry->frame * g_mmu_config->page_size;
          } else {
            // TLB miss, do page walk (use PID 0 as default for cleanup)
            uint32_t frame_number = mmu_perform_page_walk(memory_socket, page_number, 0);
            physic_addr = frame_number * g_mmu_config->page_size;
          }
        } else {
          // No TLB, do page walk directly (use PID 0 as default for cleanup)
          uint32_t frame_number = mmu_perform_page_walk(memory_socket, page_number, 0);
          physic_addr = frame_number * g_mmu_config->page_size;
        }
        mmu_request_page_write_to_memory(memory_socket, physic_addr, entry->content);
      }
      entry->is_valid = false; // Invalidate all entries
    }
    list_iterate(g_cache, _writeback_if_dirty);
    LOG_INFO("[Cache] All cache entries invalidated.");
  }
  if (g_tlb_config->entry_count > 0)
  {
    list_clean_and_destroy_elements(g_tlb, tlb_entry_destroy);
    LOG_INFO("[TLB] All TLB entries flushed.");
  }
}

void mmu_destroy()
{
  LOG_INFO("Destroying MMU resources...");
  if (g_tlb)
  {
    list_destroy_and_destroy_elements(g_tlb, tlb_entry_destroy);
  }
  if (g_cache)
  {
    list_destroy_and_destroy_elements(g_cache, cache_entry_destroy);
  }
}
/*MMU Administrativos*/

/*CACHE*/

CacheEntry *cache_find_entry(uint32_t page_number, uint32_t pid)
{

  bool _is_frame(void *element)
  {
    CacheEntry *entry = (CacheEntry *)element;
    return entry->is_valid && entry->page == page_number;
  }

  CacheEntry *entry = list_find(g_cache, _is_frame);
  if (entry)
  {
    entry->use_bit = true;
    LOG_OBLIGATORIO("PID: %d - Cache Hit - Pagina: %u", pid, page_number);
  }
  else
  {
    LOG_OBLIGATORIO("PID: %d - Cache Miss - Pagina: %u", pid, page_number);
  }
  return entry;
}

int cache_find_victim_clock()
{
  while (true)
  {
    CacheEntry *entry = list_get(g_cache, g_cache_clock_pointer);
    if (!entry->is_valid)
    {
      return g_cache_clock_pointer; // Found an unused slot
    }
    if (entry->use_bit)
    {
      entry->use_bit = false; // Give a second chance
    }
    else
    {
      // Found a victim (use_bit == 0)
      int victim_index = g_cache_clock_pointer;
      g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
      return victim_index;
    }
    g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
  }
}

int cache_find_victim_clock_m()
{
  // First pass: look for (use=0, modified=0) - best victim
  for (int i = 0; i < g_cache_config->entry_count * 2; i++)
  {
    CacheEntry *entry = list_get(g_cache, g_cache_clock_pointer);
    if (!entry->is_valid)
      return g_cache_clock_pointer;
    if (!entry->use_bit && !entry->modified_bit)
    {
      int victim_index = g_cache_clock_pointer;
      g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
      return victim_index;
    }
    g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
  }
  
  // Second pass: look for (use=0, modified=1) - need writeback
  for (int i = 0; i < g_cache_config->entry_count * 2; i++)
  {
    CacheEntry *entry = list_get(g_cache, g_cache_clock_pointer);
    if (!entry->use_bit && entry->modified_bit)
    {
      int victim_index = g_cache_clock_pointer;
      g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
      return victim_index;
    }
    entry->use_bit = false; // Give second chance
    g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
  }
  
  // If we get here, fall back to regular CLOCK algorithm
  return cache_find_victim_clock();
}

CacheEntry* select_victim_entry(int memory_socket, uint32_t logic_dir, uint32_t pid)
{
  LOG_INFO("[Cache] Loading page for logical address %u", logic_dir);
  int victim_index;
  if (g_cache_config->replacement_algo == CACHE_ALGO_CLOCK)
  {
    victim_index = cache_find_victim_clock();
  }
  else
  {
    victim_index = cache_find_victim_clock_m();
  }

  CacheEntry *victim_entry = list_get(g_cache, victim_index);

 
  
  if (victim_entry->is_valid && victim_entry->modified_bit)
  {
    LOG_INFO("[Cache] Victim (page %u) is dirty. Writing back to memory.", victim_entry->page);
    // Calculate physical address for the victim page
    uint32_t victim_page_number = victim_entry->page;
    uint32_t physic_addr;
    
    if (g_tlb_config->entry_count > 0){
      // Use TLB to get frame number for victim page
      TLBEntry* tlb_entry = tlb_find_entry(victim_page_number);
      if (tlb_entry) {
        physic_addr = tlb_entry->frame * g_mmu_config->page_size;
      } else {
        // TLB miss for victim page, do page walk
        uint32_t frame_number = mmu_perform_page_walk(memory_socket, victim_page_number, pid);
        physic_addr = frame_number * g_mmu_config->page_size;
      }
    } else {
      // No TLB, do page walk directly
      uint32_t frame_number = mmu_perform_page_walk(memory_socket, victim_page_number, pid);
      physic_addr = frame_number * g_mmu_config->page_size;
    }
    uint32_t frame_number = physic_addr / g_mmu_config->page_size;
    LOG_OBLIGATORIO("PID: %u - Memory Update - Página: %u - Frame: %u", pid, victim_entry->page, frame_number);
    mmu_request_page_write_to_memory(memory_socket, physic_addr, victim_entry->content);
  }
  return victim_entry;
}

CacheEntry* cache_load_page(uint32_t logic_dir, int memory_socket, CacheEntry *victim_entry, uint32_t pid)
{

  LOG_DEBUG("[Cache] Replacing victim entry (frame %u) with new page.", victim_entry->page);
  uint32_t page_number = logic_dir / g_mmu_config->page_size;
  uint32_t physic_dir;
  
  // Use MMU translation to get physical address
  if (g_tlb_config->entry_count > 0)
  {
    // Use TLB translation
    physic_dir = mmu_translate_address(memory_socket, logic_dir, pid);
    // Convert to page-aligned address
    physic_dir = (physic_dir / g_mmu_config->page_size) * g_mmu_config->page_size;
  }
  else
  {
    // No TLB, do page walk directly
    uint32_t frame_number = mmu_perform_page_walk(memory_socket, page_number, pid);
    physic_dir = frame_number * g_mmu_config->page_size;
  }
  
  mmu_request_page_read_from_memory(memory_socket, physic_dir, victim_entry->content);

  victim_entry->is_valid = true;
  victim_entry->page = page_number;
  victim_entry->use_bit = true;
  victim_entry->modified_bit = false;

  LOG_OBLIGATORIO("PID: %d - Cache Add - Pagina: %u", pid, page_number);
  return victim_entry;
}