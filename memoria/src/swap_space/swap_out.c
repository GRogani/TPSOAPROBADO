#include "swap_out.h"
#include "swap_manager.h"
#include "../user_space/user_space_memory.h"
#include "../user_space/frame_manager.h"

/**
 * @brief Suspend a process - move its pages from user space to swap space.
 */
int swap_out_process(uint32_t pid) {
    LOG_INFO("## PID: %u - Iniciando suspensión del proceso", pid);
    
    process_info *proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("## PID: %u - Proceso no encontrado para suspender", pid);
        return -1;
    }
    
    if (proc->is_suspended) {
        LOG_WARNING("## PID: %u - El proceso ya está suspendido", pid);
        return 0; // Already suspended, nothing to do
    }
    
    if (proc->allocated_frames == NULL || list_is_empty(proc->allocated_frames)) {
        LOG_WARNING("## PID: %u - No hay frames asignados para suspender", pid);
        proc->is_suspended = true;
        return 0;
    }

    t_list *frames_to_swap = list_create();
    for (int i = 0; i < list_size(proc->allocated_frames); i++) {
        uint32_t *frame_num = list_get(proc->allocated_frames, i);
        uint32_t *new_frame = malloc(sizeof(uint32_t));
        *new_frame = *frame_num;
        list_add(frames_to_swap, new_frame);
    }
    
    uint32_t frames_needed = list_size(frames_to_swap);
    t_list *swap_frames = swap_allocate_frames(frames_needed);
    
    if (swap_frames == NULL || list_size(swap_frames) < frames_needed) {
        LOG_ERROR("## PID: %u - No hay suficiente espacio en swap para %u páginas", pid, frames_needed);
        // Clean up
        list_destroy_and_destroy_elements(frames_to_swap, free);
        if (swap_frames != NULL) {
            list_destroy_and_destroy_elements(swap_frames, free);
        }
        return -1;
    }
    
    void *page_buffer = malloc(memoria_config.TAM_PAGINA);
    if (page_buffer == NULL) {
        LOG_ERROR("## PID: %u - Error al asignar buffer para suspensión", pid);
        list_destroy_and_destroy_elements(frames_to_swap, free);
        list_destroy_and_destroy_elements(swap_frames, free);
        return -1;
    }
    
    bool success = true;
    for (int i = 0; i < list_size(frames_to_swap) && success; i++) {
        uint32_t *user_frame = list_get(frames_to_swap, i);
        uint32_t *swap_frame = list_get(swap_frames, i);
        
        uint32_t physical_address = (*user_frame) * memoria_config.TAM_PAGINA;
        
        memset(page_buffer, 0, memoria_config.TAM_PAGINA);
        read_from_user_space(physical_address, page_buffer, memoria_config.TAM_PAGINA);
        
        if (swap_write_frame(*swap_frame, page_buffer, memoria_config.TAM_PAGINA) != 0) {
            LOG_ERROR("## PID: %u - Error al escribir frame %u en swap", pid, *swap_frame);
            success = false;
            break;
        }
        
        LOG_DEBUG("## PID: %u - Página movida: User frame %u -> Swap frame %u", pid, *user_frame, *swap_frame);
    }
    
    free(page_buffer);
    
    if (!success) {
        LOG_ERROR("## PID: %u - Fallo en la suspensión del proceso", pid);
        list_destroy_and_destroy_elements(frames_to_swap, free);
        list_destroy_and_destroy_elements(swap_frames, free);
        return -1;
    }
    
    release_frames(proc->allocated_frames);
    
    proc->allocated_frames = swap_frames;
    
    proc->is_suspended = true;
    
    proc->metrics->swap_out_count++;
    
    LOG_OBLIGATORIO("## PID: %u - Proceso suspendido exitosamente. Páginas movidas a swap: %d", pid, list_size(swap_frames));
    
    list_destroy_and_destroy_elements(frames_to_swap, free);
    
    return 0;
}
