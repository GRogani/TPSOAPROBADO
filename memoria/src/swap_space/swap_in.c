#include "swap_in.h"
#include "swap_manager.h"
#include "../user_space/user_space_memory.h"
#include "../user_space/frame_manager.h"
#include "../semaphores.h"

/**
 * @brief Resume a process - move its pages from swap space back to user space.
 */
int swap_in_process(uint32_t pid) {
    LOG_INFO("## PID: %u - Iniciando reanudación del proceso", pid);
    
    lock_swap_file();
    
    process_info *proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("## PID: %u - Proceso no encontrado para reanudar", pid);
        unlock_swap_file();
        return -1;
    }
    
    if (!proc->is_suspended) {
        LOG_WARNING("## PID: %u - El proceso no está suspendido", pid);
        unlock_swap_file();
        return 0; // Already in memory, nothing to do
    }
    
    if (proc->allocated_frames == NULL || list_is_empty(proc->allocated_frames)) {
        LOG_WARNING("## PID: %u - No hay frames en swap para reanudar", pid);
        proc->is_suspended = false;
        unlock_swap_file();
        return 0;
    }

    t_list *frames_to_unswap = list_create();
    uint32_t allocated_frames_length = list_size(proc->allocated_frames);
    for (int i = 0; i < allocated_frames_length; i++)
    {
        uint32_t *frame_num = list_get(proc->allocated_frames, i);
        uint32_t *new_frame = malloc(sizeof(uint32_t));
        *new_frame = *frame_num;
        list_add(frames_to_unswap, new_frame);
    }

    uint32_t frames_needed = list_size(frames_to_unswap);
    t_list *user_frames = allocate_frames(frames_needed);
    
    if (user_frames == NULL || list_size(user_frames) < frames_needed) {
        LOG_ERROR("## PID: %u - No hay suficiente espacio en memoria para %u páginas", pid, frames_needed);
        list_destroy_and_destroy_elements(frames_to_unswap, free);
        if (user_frames != NULL) {
            list_destroy_and_destroy_elements(user_frames, free);
        }
        unlock_swap_file();
        return -1;
    }
    
    void *page_buffer = malloc(memoria_config.TAM_PAGINA);
    if (page_buffer == NULL) {
        LOG_ERROR("## PID: %u - Error al asignar buffer para reanudación", pid);
        list_destroy_and_destroy_elements(frames_to_unswap, free);
        list_destroy_and_destroy_elements(user_frames, free);
        unlock_swap_file();
        return -1;
    }
    
    bool success = true;
    for (int i = 0; i < list_size(frames_to_unswap) && success; i++) {
        uint32_t *swap_frame = list_get(frames_to_unswap, i);
        uint32_t *user_frame = list_get(user_frames, i);
        
        uint32_t physical_address = (*user_frame) * memoria_config.TAM_PAGINA;
        
        memset(page_buffer, 0, memoria_config.TAM_PAGINA);
        int success = swap_read_frame(*swap_frame, page_buffer, memoria_config.TAM_PAGINA) == 0;
        if (!success) {
            LOG_ERROR("## PID: %u - Error al leer frame %u de swap", pid, *swap_frame);
            success = false;
            break;
        }
        
        write_to_user_space(physical_address, page_buffer, memoria_config.TAM_PAGINA);
        
        LOG_DEBUG("## PID: %u - Página movida: Swap frame %u -> User frame %u", pid, *swap_frame, *user_frame);
    }
    
    free(page_buffer);
    
    if (!success) {
        LOG_ERROR("## PID: %u - Fallo en la reanudación del proceso", pid);
        list_destroy_and_destroy_elements(frames_to_unswap, free);
        release_frames(user_frames);
        unlock_swap_file();
        return -1;
    }
    
    swap_release_frames(proc->allocated_frames);
    
    proc->allocated_frames = user_frames;
    proc->is_suspended = false;
    proc->metrics->swap_in_count++;
    
    LOG_OBLIGATORIO("## PID: %u - Proceso reanudado exitosamente. Páginas movidas desde swap: %d", pid, list_size(user_frames));
    
    list_destroy_and_destroy_elements(frames_to_unswap, free);
    
    unlock_swap_file();
    return 0;
}
