#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "../utils.h"
#include "structures/page_table.h"

// Global memory space pointer (to be allocated at init)
extern void* user_memory_space;
extern size_t MEMORY_SIZE;
extern int PAGE_SIZE; // From config, useful for memory operations

/**
 * @brief Initializes the main user memory space.
 * @param config A pointer to the t_memoria_config structure.
 * @return True if initialization is successful, false otherwise.
 */
bool init_user_memory(const t_memoria_config* config);

/**
 * @brief Reads a specified number of bytes from the user memory space at a given physical address.
 * @param physical_address The physical address (offset from user_memory_space start) to read from.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param size The number of bytes to read.
 * @return True if the read operation is successful and within bounds, false otherwise.
 */
bool read_memory(uint32_t physical_address, void* buffer, size_t size);

/**
 * @brief Writes a specified number of bytes to the user memory space at a given physical address.
 * @param physical_address The physical address (offset from user_memory_space start) to write to.
 * @param data A pointer to the data to be written.
 * @param size The number of bytes to write.
 * @return True if the write operation is successful and within bounds, false otherwise.
 */
bool write_memory(uint32_t physical_address, const void* data, size_t size);

/**
 * @brief Reads an entire page from the user memory space starting from the given physical address.
 * Assumes the physical_address is page-aligned (0th byte of the page).
 * @param physical_address The page-aligned physical address to read from.
 * @param buffer A pointer to the buffer where the page content will be stored. The buffer must be large enough to hold a full page (PAGE_SIZE).
 * @return True if the read operation is successful and within bounds, false otherwise.
 */
bool read_full_page(uint32_t physical_address, void* buffer);

/**
 * @brief Writes an entire page to the user memory space starting from the given physical address.
 * Assumes the physical_address is page-aligned (0th byte of the page).
 * @param physical_address The page-aligned physical address to write to.
 * @param data A pointer to the data representing the full page content. The data must be PAGE_SIZE bytes long.
 * @return True if the write operation is successful and within bounds, false otherwise.
 */
bool update_full_page(uint32_t physical_address, const void* data);

/**
 * @brief Frees the allocated user memory space.
 */
void memory_manager_destroy();

#endif