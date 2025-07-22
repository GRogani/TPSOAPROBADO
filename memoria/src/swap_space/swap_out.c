#include "swap_out.h"

bool swap_out_process(int32_t pid)
{
    LOG_INFO("## PID: %u - Iniciando suspensión del proceso", pid);

    lock_swap_file();

    process_info *proc = process_manager_find_process(pid);
    if (proc == NULL)
    {
        LOG_ERROR("## PID: %u - Proceso no encontrado para suspender", pid);
        unlock_swap_file();
        return false;
    }

    if (proc->is_suspended)
    {
        LOG_WARNING("## PID: %u - El proceso ya está suspendido", pid);
        unlock_swap_file();
        return true;
    }

    if (proc->allocated_frames == NULL)
    {
        LOG_WARNING("## PID: %u - No hay frames asignados para suspender", pid);
        proc->is_suspended = true;
        unlock_swap_file();
        return true;
    }

    int32_t allocated_frames_length = list_size(proc->allocated_frames);

    t_list *frames_to_swap = list_create();
    for (int i = 0; i < allocated_frames_length; i++)
    {
        int32_t *frame_num = list_get(proc->allocated_frames, i);
        int32_t *new_frame = malloc(sizeof(int32_t));
        *new_frame = *frame_num;
        list_add(frames_to_swap, new_frame);
    }

    int32_t frames_needed = list_size(frames_to_swap);
    t_list *swap_frames = swap_allocate_frames(frames_needed);

    if (swap_frames == NULL || list_size(swap_frames) < frames_needed)
    {
        LOG_ERROR("## PID: %u - No hay suficiente espacio en swap para %u páginas", pid, frames_needed);
        // Clean up
        list_destroy_and_destroy_elements(frames_to_swap, free);
        if (swap_frames != NULL)
        {
            list_destroy_and_destroy_elements(swap_frames, free);
        }
        unlock_swap_file();
        return false;
    }

    void *page_buffer = malloc(memoria_config.TAM_PAGINA);
    if (page_buffer == NULL)
    {
        LOG_ERROR("## PID: %u - Error al asignar buffer para suspensión", pid);
        list_destroy_and_destroy_elements(frames_to_swap, free);
        list_destroy_and_destroy_elements(swap_frames, free);
        unlock_swap_file();
        return false;
    }

    bool success = true;
    for (int i = 0; i < list_size(frames_to_swap) && success; i++)
    {
        int32_t *user_frame = list_get(frames_to_swap, i);
        int32_t *swap_frame = list_get(swap_frames, i);

        int32_t physical_address = (*user_frame) * memoria_config.TAM_PAGINA;

        memset(page_buffer, 0, memoria_config.TAM_PAGINA);
        read_from_user_space(physical_address, page_buffer, memoria_config.TAM_PAGINA);

        if (!swap_write_frame(*swap_frame, page_buffer, memoria_config.TAM_PAGINA))
        {
            LOG_ERROR("## PID: %u - Error al escribir frame %u en swap", pid, *swap_frame);
            success = false;
            break;
        }

        LOG_DEBUG("## PID: %u - Página movida: User frame %u -> Swap frame %u", pid, *user_frame, *swap_frame);
    }

    free(page_buffer);

    if (!success)
    {
        LOG_ERROR("## PID: %u - Fallo en la suspensión del proceso", pid);
        list_destroy_and_destroy_elements(frames_to_swap, free);
        list_destroy_and_destroy_elements(swap_frames, free);
        unlock_swap_file();
        return false;
    }

    release_frames(proc->allocated_frames);

    proc->allocated_frames = swap_frames;

    proc->is_suspended = true;

    proc->metrics->swap_out_count++;

    LOG_OBLIGATORIO("## PID: %u - Proceso suspendido exitosamente. Páginas movidas a swap: %d", pid, list_size(swap_frames));

    list_destroy_and_destroy_elements(frames_to_swap, free);

    unlock_swap_file();
    return true;
}
