#ifndef MEMORIA_COLLECTIONS_H
#define MEMORIA_COLLECTIONS_H

#include <commons/collections/list.h>
#include "../utils.h"

// Constante para frame number inválido
#define INVALID_FRAME_NUMBER -1

// Page Table structure (definir primero)
typedef struct page_table {
    uint32_t table_id; // Unique identifier for the page table
    t_list* entries;
    size_t num_entries;
} t_page_table;

// Page Table Entry structure
typedef struct page_table_entry
{
    bool is_last_level;
    uint32_t next_table_id;    // ID de la siguiente tabla (si is_last_level == false)
    union {
        int32_t frame_number;  // Número de frame (si is_last_level == true)
        t_page_table* next_table;  // Puntero a la siguiente tabla (si is_last_level == false)
    };
} t_page_table_entry;

// Function declarations for Page Table management

/**
 * @brief Creates and initializes a new t_page_table structure.
 * @return A pointer to the newly created t_page_table, or NULL if allocation fails.
 */
t_page_table* create_page_table();

/**
 * @brief Creates and initializes a new t_page_table_entry.
 * @param is_last_level A boolean indicating if this entry is for the last level of the page table (points to a frame) or an intermediate level (points to another page table).
 * @return A pointer to the newly created t_page_table_entry, or NULL if allocation fails.
 */
t_page_table_entry* create_page_table_entry(bool is_last_level);

/**
 * @brief Adds a page table entry to a page table at a specific index.
 * Note: For fixed-size tables, consider pre-filling or using list_replace.
 * @param page_table The page table to which the entry will be added.
 * @param index The index at which to add the entry.
 * @param entry The page table entry to add.
 */
void add_page_table_entry(t_page_table* page_table, int index, t_page_table_entry* entry);

/**
 * @brief Retrieves a page table entry from a page table by its index.
 * @param page_table The page table from which to retrieve the entry.
 * @param index The index of the entry to retrieve.
 * @return A pointer to the t_page_table_entry at the specified index, or NULL if the index is out of bounds or pointers are invalid.
 */
t_page_table_entry* get_page_table_entry(t_page_table* page_table, int index);

/**
 * @brief Destroys a t_page_table_entry and any nested page tables it points to.
 * This function is intended to be used as the element_destroyer for list_destroy_and_destroy_elements.
 * @param entry_void_ptr A void pointer to the t_page_table_entry to be destroyed.
 */
void destroy_page_table_entry(void* entry_void_ptr);

/**
 * @brief Destroys a t_page_table and all its contained entries, including any nested page tables.
 * @param page_table A pointer to the t_page_table to be destroyed.
 */
void destroy_page_table(t_page_table* page_table);

/**
 * @brief Initializes a complete multi-level hierarchical page table based on the provided configuration.
 * @param config A pointer to the t_memoria_config structure containing CANTIDAD_NIVELES and ENTRADAS_POR_TABLA.
 * @return A pointer to the root t_page_table of the initialized hierarchy, or NULL if an error occurs.
 */
t_page_table* init_page_table(const t_memoria_config* config);

/**
 * @brief Resets all frame numbers in a multi-level page table to invalid values (-1).
 * This is used before reassigning frames during swap in.
 * @param current_table The page table to reset.
 * @param total_levels Total number of levels in the page table hierarchy.
 * @param current_level Current level being processed (should start with 1).
 * @return true if successful, false otherwise.
 */
bool reset_page_table_frames(t_page_table* current_table, int total_levels, int current_level);

#endif