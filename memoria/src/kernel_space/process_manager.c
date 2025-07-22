#include "process_manager.h"

void destroy_process_info(void *proc_void_ptr)
{
    process_info *proc = (process_info *)proc_void_ptr;
    if (proc == NULL)
        return;

    LOG_INFO("## PID: %u - Liberando recursos del proceso.", proc->pid);

    LOG_OBLIGATORIO("## PID: %u - Proceso Destruido - Métricas - Acc.T.Pag: %u; Inst.Sol.: %u; SWAP: %u; Mem.Prin.: %u; Lec.Mem.: %u; Esc.Mem.: %u",
                    proc->pid,
                    proc->metrics->page_table_access_count,
                    proc->metrics->instruction_requests_count,
                    proc->metrics->swap_out_count,
                    proc->metrics->swap_in_count,
                    proc->metrics->memory_read_count,
                    proc->metrics->memory_write_count);

    if (proc->process_size != 0)
    {

        if (proc->instructions != NULL)
        {
            list_destroy_and_destroy_elements(proc->instructions, free);
        }

        if (proc->page_table != NULL)
        {
            free_page_table(proc->page_table);
        }

        if (proc->allocated_frames != NULL)
        {
            release_frames(proc->allocated_frames);
        }
    }

    if (proc->metrics != NULL)
    {
        free(proc->metrics);
    }

    free(proc);
}

void process_manager_init()
{
    global_process_list = list_create();
    if (global_process_list == NULL)
    {
        LOG_ERROR("Process Manager: Fallo al crear la lista global de procesos(PIDs).");
        exit(1);
    }
    LOG_INFO("Process Manager: Inicializado.");
}

void process_manager_destroy()
{
    list_destroy_and_destroy_elements(global_process_list, destroy_process_info);
    LOG_INFO("Process Manager: Destruido.");
}

process_info *process_manager_find_process(int32_t pid)
{
    process_info *found_proc = NULL;
    lock_process_list();
    for (int i = 0; i < list_size(global_process_list); i++)
    {
        process_info *proc = list_get(global_process_list, i);
        if (proc->pid == pid)
        {
            found_proc = proc;
            break;
        }
    }
    unlock_process_list();
    return found_proc;
}

bool process_manager_delete_process(int32_t pid)
{
    bool result = false;
    lock_process_list();
    int found_index = -1;
    for (int i = 0; i < list_size(global_process_list); i++)
    {
        process_info *proc = list_get(global_process_list, i);
        if (proc->pid == pid)
        {
            found_index = i;
            break;
        }
    }

    if (found_index > -1)
    {
        list_remove_and_destroy_element(global_process_list, found_index, destroy_process_info);
        LOG_INFO("## PID: %u - Proceso Finalizado y recursos liberados.", pid);
        result = true;
    }
    else
    {
        LOG_ERROR("## PID: %u - Intento de finalizar proceso no existente.", pid);
    }
    unlock_process_list();
    return result;
}

t_list *process_manager_get_process_list()
{
    return global_process_list;
}