#include "mmu.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "delay_utils.h"
#include "utils/safe_alloc.h"
#include "utils/serialization/package.h"
#include "utils/DTPs/mmu_request_page_read_response.h"
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

int32_t mmu_request_pagetable_entry_from_memory(int memory_socket, int32_t table_id, int32_t entry_index, int32_t pid)
{
  LOG_DEBUG("[MEM-REQUEST] Requesting page table entry from memory. PID: %u, Table ID: %u, Entry Index: %u", pid, table_id, entry_index);
  send_page_entry_request_package(memory_socket, pid, table_id, entry_index);

  // Wait for response from memory
  t_package *response_package = recv_package(memory_socket);
  if (response_package == NULL || response_package->opcode != GET_PAGE_ENTRY)
  {
    LOG_ERROR("[MEM-REQUEST] Failed to receive valid response for page table entry request");
    if (response_package)
      destroy_package(response_package);
    return 0; // Error
  }

  // Parse the response
  page_entry_response_data response = read_page_entry_response_package(response_package);
  destroy_package(response_package);

  LOG_DEBUG("[MEM-REQUEST] Received value %u from memory", response.value);
  return response.value;
}

void mmu_request_page_read_from_memory(int memory_socket, int32_t physic_addr, void *buffer)
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

void mmu_request_page_write_to_memory(int memory_socket, int32_t physic_addr, void *content)
{
  LOG_INFO("[MEM-REQUEST] Telling Memory to write page to physic addres %u", physic_addr);
  int32_t content_size = g_mmu_config->page_size;
  t_memory_write_request *request = create_memory_write_request(physic_addr, content_size, content);
  send_memory_write_request(memory_socket, request);
  destroy_memory_write_request(request);
  t_package *package = recv_package(memory_socket);
  if (package->opcode != CONFIRMATION)
  {
    LOG_ERROR("Failed to receive confirmation package from memory");
  }
  bool success = read_confirmation_package(package);
  if (!success)
  {
    LOG_ERROR("Failed to write cache in memory");
    destroy_package(package);
  }
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

TLBEntry *tlb_find_entry(int32_t page_number, int32_t pid)
{
  if (g_tlb_config->entry_count == 0)
    return NULL;

  bool _is_page(void *element)
  {
    TLBEntry *entry = (TLBEntry *)element;
    return entry->page == page_number && entry->pid == pid;
  }

  TLBEntry *entry = list_find(g_tlb, _is_page);

  if (entry != NULL && g_tlb_config->replacement_algo == TLB_ALGO_LRU)
  {
    entry->lru_timestamp = g_lru_timestamp_counter++;
  }
  return entry;
}

void tlb_add_entry(int32_t page_number, int32_t frame_number, int32_t pid)
{
  if (g_tlb_config->entry_count == 0)
    return;

  TLBEntry *new_entry = malloc(sizeof(TLBEntry));
  new_entry->page = page_number;
  new_entry->frame = frame_number;
  new_entry->pid = pid;
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
    //TLBEntry *victim = list_get(g_tlb, victim_index);
    //LOG_DEBUG("[TLB] Replacing victim (page %u) at index %d with new entry (page %u).", victim->page, victim_index, page_number);
    list_replace_and_destroy_element(g_tlb, victim_index, new_entry, tlb_entry_destroy);
  }
}

int32_t mmu_perform_page_walk(int memory_socket, int32_t page_number, int32_t pid)
{
  LOG_INFO("[MMU] Page Walk starting for page number %u...", page_number);

  int32_t next_table_id = 1;

  for (int level = 1; level <= g_mmu_config->page_table_levels; level++)
  {

    int power = g_mmu_config->page_table_levels - level;
    int32_t divisor = (int32_t)pow(g_mmu_config->entries_per_table, power);
    int32_t entry_index = (int32_t)floor(page_number / divisor) % g_mmu_config->entries_per_table;

    LOG_DEBUG("  - Level %d: Power %d, Divisor %u -> table entry %u",
              level, power, divisor, entry_index);

    next_table_id = mmu_request_pagetable_entry_from_memory(memory_socket, next_table_id, entry_index, pid);
    if (next_table_id == 0)
    {
      LOG_ERROR("[MMU] Failed to get page table entry for level %d, entry %u", level, entry_index);
      return 0; // Error
    }

    LOG_DEBUG("    Next table/frame ID: %u", next_table_id);
  }

  int32_t frame_number = next_table_id;
  LOG_OBLIGATORIO("PID: %d - OBTENER MARCO - Página: %u - Marco: %u", pid, page_number, frame_number);
  return frame_number;
}

int32_t mmu_translate_address(int memory_socket, int32_t logical_address, int32_t pid)
{

  int32_t page_number = floor(logical_address / g_mmu_config->page_size);
  int32_t offset = logical_address % g_mmu_config->page_size;

  LOG_INFO("--- Translating Logical Address %u ---", logical_address);
  LOG_DEBUG("  -> Page Number: %u, Offset: %u", page_number, offset);

  int32_t frame_number;
  TLBEntry *tlb_entry = tlb_find_entry(page_number, pid);

  if (tlb_entry)
  {
    LOG_OBLIGATORIO("PID: %d - TLB HIT - Pagina: %u", pid, page_number);
    frame_number = tlb_entry->frame;
  }
  else
  {
    LOG_OBLIGATORIO("PID: %d - TLB MISS - Pagina: %u", pid, page_number);
    // TODO: find victim entry in TLB
    frame_number = mmu_perform_page_walk(memory_socket, page_number, pid);
    tlb_add_entry(page_number, frame_number, pid);
  }

  int32_t physical_address = (frame_number * g_mmu_config->page_size) + offset;
  LOG_DEBUG("  -> Resulting Physical Address: %u", physical_address);

  return physical_address;
}

int32_t mmu_translate_address_with_page_number(int memory_socket, int32_t page_number, int32_t pid)
{
  int32_t frame_number;
  TLBEntry *tlb_entry = tlb_find_entry(page_number, pid);

  if (tlb_entry)
  {
    LOG_OBLIGATORIO("PID: %d - TLB HIT - Pagina: %u", pid, page_number);
    frame_number = tlb_entry->frame;
  }
  else
  {
    LOG_OBLIGATORIO("PID: %d - TLB MISS - Pagina: %u", pid, page_number);
    frame_number = mmu_perform_page_walk(memory_socket, page_number, pid);
    tlb_add_entry(page_number, frame_number, pid);
  }

  return (frame_number * g_mmu_config->page_size);
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
    // TODO: agregar el limite de entradas de TLB.
  }
  if (g_cache_config->entry_count > 0)
  {
    g_cache = list_create();
    for (int i = 0; i < g_cache_config->entry_count; i++)
    {
      CacheEntry *entry = malloc(sizeof(CacheEntry));
      entry->is_valid = false;
      entry->content = safe_calloc(1, g_mmu_config->page_size); // Initialize with zeros
      entry->modified_start = 0;
      entry->modified_end = 0;
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
        int32_t physic_dir = mmu_translate_address_with_page_number(memory_socket, entry->page, entry->pid);

        if (entry->modified_start < entry->modified_end)
        {
          int32_t modified_region_addr = physic_dir + entry->modified_start;
          int32_t modified_size = entry->modified_end - entry->modified_start;

          LOG_INFO("[Cache] Writing modified region to memory during cleanup: page %u, offset %u to %u (size %u bytes)",
                   entry->page, entry->modified_start, entry->modified_end, modified_size);

          t_memory_write_request *request = create_memory_write_request(
              modified_region_addr,
              modified_size,
              ((char *)entry->content) + entry->modified_start);

          send_memory_write_request(memory_socket, request);
          destroy_memory_write_request(request);

          t_package *package = recv_package(memory_socket);
          if (package->opcode != CONFIRMATION)
          {
            LOG_ERROR("Failed to receive confirmation for memory write during cleanup");
          }
          else
          {
            bool success = read_confirmation_package(package);
            if (!success)
            {
              LOG_ERROR("Memory write operation failed during cleanup");
            }
            destroy_package(package);
          }
        }
        else
        {
          LOG_INFO("[Cache] No specific modified region or invalid tracking for page %u. Skipping memory write during cleanup.", entry->page);
        }
      }

      entry->is_valid = false;
      entry->modified_bit = false;
      entry->modified_start = 0;
      entry->modified_end = 0;
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

CacheEntry *cache_find_entry(int32_t page_number, int32_t pid)
{
  delay_cache_access();

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
      // Found an unused slot, update pointer
      int32_t victim_index = g_cache_clock_pointer;
      g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
      return victim_index;
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
  while (true)
  {
    // Paso 1: Buscar (uso=0, modificado=0).
    for (int i = 0; i < g_cache_config->entry_count; i++)
    {
      CacheEntry *entry = list_get(g_cache, g_cache_clock_pointer);
      if (!entry->is_valid)
      {
        // Se encontró un espacio libre, es la mejor víctima.
        int32_t victim_index = g_cache_clock_pointer;
        g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
        return victim_index;
      }
      if (!entry->use_bit && !entry->modified_bit)
      {
        int victim_index = g_cache_clock_pointer;
        g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
        return victim_index;
      }
      g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
    }

    // Paso 2: Buscar (uso=0, modificado=1) y poner bit de uso en 0.
    for (int i = 0; i < g_cache_config->entry_count; i++)
    {
      CacheEntry *entry = list_get(g_cache, g_cache_clock_pointer);
      if (entry->is_valid)
      {
        if (!entry->use_bit && entry->modified_bit)
        {
          int victim_index = g_cache_clock_pointer;
          g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
          return victim_index;
        }
        entry->use_bit = false;
      }
      g_cache_clock_pointer = (g_cache_clock_pointer + 1) % g_cache_config->entry_count;
    }
    // El ciclo se repite, garantizando encontrar una víctima en la siguiente vuelta.
  }
}

CacheEntry *select_victim_entry(int memory_socket, int32_t pid)
{
  LOG_INFO("[Cache] Loading page into cache for PID %u", pid);
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

  int32_t physic_addr = 0;

  if (victim_entry->is_valid && victim_entry->modified_bit)
  {
    LOG_INFO("[Cache] Victim (page %u) is dirty. Writing back to memory.", victim_entry->page);

    if (g_tlb_config->entry_count > 0)
    {
      physic_addr = mmu_translate_address_with_page_number(memory_socket, victim_entry->page, victim_entry->pid);
      if (physic_addr == 0)
      {
        LOG_ERROR("[MMU] Failed to translate address for victim entry page %u, PID %u", victim_entry->page, victim_entry->pid);
        return NULL; // Error
      }
      int32_t frame_number = (physic_addr) / g_mmu_config->page_size;
      LOG_OBLIGATORIO("PID: %u - Memory Update - Página: %u - Frame: %u", victim_entry->pid, victim_entry->page, frame_number);
    }
    else
    {
      int32_t frame_number = mmu_perform_page_walk(memory_socket, victim_entry->page, victim_entry->pid);
      physic_addr = (frame_number * g_mmu_config->page_size);
      LOG_OBLIGATORIO("PID: %u - Memory Update - Página: %u - Frame: %u", victim_entry->pid, victim_entry->page, frame_number);
    }

    if (victim_entry->modified_start < victim_entry->modified_end)
    {
      int32_t modified_region_addr = physic_addr + victim_entry->modified_start;
      int32_t modified_size = victim_entry->modified_end - victim_entry->modified_start;

      LOG_INFO("[Cache] Writing modified region to memory: offset %u to %u (size %u bytes)",
               victim_entry->modified_start, victim_entry->modified_end, modified_size);

      t_memory_write_request *request = create_memory_write_request(
          modified_region_addr,
          modified_size,
          ((char *)victim_entry->content) + victim_entry->modified_start);

      send_memory_write_request(memory_socket, request);
      destroy_memory_write_request(request);

      t_package *package = recv_package(memory_socket);
      if (package->opcode != CONFIRMATION)
      {
        LOG_ERROR("Failed to receive confirmation for memory write");
      }
      else
      {
        bool success = read_confirmation_package(package);
        if (!success)
        {
          LOG_ERROR("Memory write operation failed");
        }
        destroy_package(package);
      }
    }
    else
    {
      LOG_INFO("[Cache] No specific modified region or invalid tracking. Skipping memory write.");
    }

    memset(victim_entry->content, 0, g_mmu_config->page_size);
    victim_entry->modified_start = 0;
    victim_entry->modified_end = 0;
  }
  return victim_entry;
}

CacheEntry *cache_load_page(int32_t logic_dir, int memory_socket, CacheEntry *victim_entry, int32_t pid)
{

  LOG_DEBUG("[Cache] Replacing victim entry (frame %u) with new page.", victim_entry->page);
  int32_t frame_number;
  int32_t page_number = floor(logic_dir / g_mmu_config->page_size);
  int32_t offset = logic_dir % g_mmu_config->page_size;
  int32_t physic_dir = 0;
  if (g_tlb_config->entry_count > 0)
  {
    physic_dir = mmu_translate_address(memory_socket, logic_dir, pid);
    frame_number = (physic_dir - offset) / g_mmu_config->page_size;
  }
  else
  {
    frame_number = mmu_perform_page_walk(memory_socket, page_number, pid);
    physic_dir = (frame_number * g_mmu_config->page_size) + offset;
  }

  // antes de asignar la nueva página, limpiar el contenido del victim_entry
  memset(victim_entry->content, 0, g_mmu_config->page_size);

  mmu_request_page_read_from_memory(memory_socket, physic_dir, victim_entry->content);

  victim_entry->is_valid = true;
  victim_entry->page = page_number;
  victim_entry->pid = pid; // Ensure we set the pid
  victim_entry->use_bit = true;
  victim_entry->modified_bit = false;
  victim_entry->modified_start = 0;
  victim_entry->modified_end = 0;

  LOG_OBLIGATORIO("PID: %d - Cache Add - Pagina: %u", pid, page_number);
  LOG_OBLIGATORIO("PID: %d - Acción: LEER - Dirección Física: %u - Valor: %s", pid, physic_dir, (char*)victim_entry->content);

  return victim_entry;
}
