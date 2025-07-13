// /**
//  * @brief Obtiene la cantidad actual de páginas libres en el archivo de swap.
//  * @return La cantidad de páginas libres.
//  */
// size_t swap_get_free_pages_count(void) {
//     size_t free_pages = 0;
    
//     pthread_mutex_lock(&blocks_mutex);
    
//     for (int i = 0; i < list_size(swap_blocks); i++) {
//         t_swap_block* block = list_get(swap_blocks, i);
//         if (block && block->is_used == 0) {
//             free_pages += block->num_pages;
//         }
//     }
    
//     pthread_mutex_unlock(&blocks_mutex);
    
//     return free_pages;
// }

// /**
//  * @brief Libera memoria asociada a una estructura de información de proceso en swap
//  * @param element Puntero a la estructura a liberar
//  */
// void free_swap_process_info(void* element) {
//     if (element) {
//         t_swap_page_info* info = (t_swap_page_info*)element;
//         free(info);
//     }
// }
