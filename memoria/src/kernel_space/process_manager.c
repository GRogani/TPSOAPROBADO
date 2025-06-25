#include "process_manager.h"

static t_list* global_process_list = NULL;

extern t_memoria_config memoria_config;

// Helper to recursively assign allocated frames to the last level page table entries
static bool assign_frames_to_page_table(t_page_table* current_table, t_list* allocated_frames_for_process, int* current_frame_index, int total_levels, int current_level) {
    if (current_table == NULL || allocated_frames_for_process == NULL) {
        LOG_ERROR("Error: Tabla actual o lista de frames es NULL en assign_frames_to_page_table.");
        return false;
    }

    bool is_last_level_of_hierarchy = (current_level == total_levels);

    for (int i = 0; i < current_table->num_entries; i++) {
        t_page_table_entry* entry = get_page_table_entry(current_table, i);
        if (entry == NULL) {
            LOG_ERROR("Error: Entrada de tabla de paginas NULL en indice %d de nivel %d.", i, current_level);
            return false;
        }

        if (is_last_level_of_hierarchy) {
            if (*current_frame_index >= list_size(allocated_frames_for_process)) {
                LOG_ERROR("Error: No hay suficientes frames asignados para todas las entradas de ultimo nivel.");
                return false;
            }
            uint32_t* frame_num_ptr = (uint32_t*)list_get(allocated_frames_for_process, *current_frame_index);
            entry->frame_number = *frame_num_ptr;
            (*current_frame_index)++;
        } else {
            t_page_table next_level_table_mock;
            next_level_table_mock.entries = entry->next_level;
            next_level_table_mock.num_entries = list_size(entry->next_level);

            if (!assign_frames_to_page_table(&next_level_table_mock, allocated_frames_for_process, current_frame_index, total_levels, current_level + 1)) {
                return false;
            }
        }
    }
    return true;
}

// Helper to destroy a process_info structure.
void destroy_process_info(void* proc_void_ptr) {
    process_info* proc = (process_info*) proc_void_ptr;
    if (proc == NULL) return;

    LOG_INFO("## PID: %u - Liberando recursos del proceso.", proc->pid);

    LOG_INFO("## PID: %u - Metricas Finales:", proc->pid);
    LOG_INFO(" - Accesos a Tablas de Paginas: %u", proc->metrics->page_table_access_count);
    LOG_INFO(" - Instrucciones Solicitadas: %u", proc->metrics->instruction_requests_count);
    LOG_INFO(" - Bajadas a SWAP: %u", proc->metrics->swap_out_count);
    LOG_INFO(" - Subidas a Memoria Principal: %u", proc->metrics->swap_in_count);
    LOG_INFO(" - Lecturas de Memoria: %u", proc->metrics->memory_read_count);
    LOG_INFO(" - Escrituras de Memoria: %u", proc->metrics->memory_write_count);

    if (proc->instructions) {
        list_destroy_and_destroy_elements(proc->instructions, free);
    }

    if (proc->page_table) {
        destroy_page_table(proc->page_table);
    }

    if (proc->allocated_frames) {
        frame_free_frames(proc->allocated_frames);
    }

    if (proc->metrics) {
        free(proc->metrics);
    }

    free(proc);
}


void process_manager_init() {
    global_process_list = list_create();
    if (global_process_list == NULL) {
        LOG_ERROR("Process Manager: Fallo al crear la lista global de procesos.");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Process Manager: Inicializado.");
}

void process_manager_destroy() {
    list_destroy_and_destroy_elements(global_process_list, destroy_process_info);
    LOG_INFO("Process Manager: Destruido.");
}


t_list* process_manager_load_script_lines(char* path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "program/%s", path);

    LOG_INFO("Intentando abrir archivo de pseudocodigo: %s", full_path);

    FILE *file = fopen(full_path, "r");
    if (!file) {
        LOG_ERROR("Error al abrir archivo: %s", full_path);
        return NULL;
    }

    LOG_INFO("Archivo de pseudocodigo abierto correctamente: %s", full_path);

    t_list* list = list_create();
    char* line = NULL;
    size_t len = 0;
    ssize_t read_bytes;

    while ((read_bytes = getline(&line, &len, file)) != -1) {
        size_t line_len = strlen(line);
        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r' ||
                                 line[line_len - 1] == ' ' || line[line_len - 1] == '\t')) {
            line[line_len - 1] = '\0';
            line_len--;
        }

        if (line_len > 0)
            list_add(list, strdup(line));
    }

    free(line);
    fclose(file);
    return list;
}

int process_manager_create_process(uint32_t pid, uint32_t size, char* script_path) {
    process_info* proc = malloc(sizeof(process_info));
    if (proc == NULL) {
        LOG_ERROR("## PID: %u - Error: No se pudo asignar memoria para process_info.", pid);
        return -1;
    }

    proc->pid = pid;
    proc->process_size = size;
    proc->is_suspended = false;
    lock_process_instructions(); // Lock if instructions can be concurrently accessed
    proc->instructions = process_manager_load_script_lines(script_path);
    unlock_process_instructions(); // Unlock

    if (script_path && strcmp(script_path, "") != 0 && proc->instructions == NULL) {
        LOG_ERROR("## PID: %u - Error al cargar pseudocodigo desde: %s", pid, script_path);
        free(proc);
        return -1;
    }

    size_t pages_needed = (size_t)ceil((double)size / memoria_config.TAM_PAGINA);
    LOG_INFO("## PID: %u - Proceso requiere %zu paginas (tamano: %u bytes, pagina: %d bytes).", pid, pages_needed, size, memoria_config.TAM_PAGINA);

    if (frame_get_free_count() < pages_needed) {
        LOG_ERROR("## PID: %u - No hay suficientes frames disponibles para crear el proceso. (Necesita %zu, Libre %zu)", pid, pages_needed, frame_get_free_count());
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        free(proc);
        return -1;
    }

    proc->allocated_frames = frame_allocate_frames(pages_needed);
    if (proc->allocated_frames == NULL || list_size(proc->allocated_frames) != pages_needed) {
        LOG_ERROR("## PID: %u - Error al asignar frames para el proceso.", pid);
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        free(proc);
        return -1;
    }
    // No logging of frames asignados as per instructions.

    proc->page_table = init_page_table(&memoria_config);
    if (proc->page_table == NULL) {
        LOG_ERROR("## PID: %u - Error al inicializar estructura de tabla de paginas.", pid);
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        frame_free_frames(proc->allocated_frames);
        free(proc);
        return -1;
    }

    int current_frame_idx = 0;
    if (!assign_frames_to_page_table(proc->page_table, proc->allocated_frames, &current_frame_idx, memoria_config.CANTIDAD_NIVELES, 1)) {
        LOG_ERROR("## PID: %u - Error al asignar frames a las entradas de la tabla de paginas.", pid);
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        destroy_page_table(proc->page_table);
        frame_free_frames(proc->allocated_frames);
        free(proc);
        return -1;
    }

    proc->metrics = malloc(sizeof(t_process_metrics));
    if (proc->metrics == NULL) {
        LOG_ERROR("## PID: %u - Error: No se pudo asignar memoria para metricas.", pid);
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        destroy_page_table(proc->page_table);
        frame_free_frames(proc->allocated_frames);
        free(proc);
        return -1;
    }
    memset(proc->metrics, 0, sizeof(t_process_metrics));

    lock_process_list();
    list_add(global_process_list, proc);
    unlock_process_list();

    LOG_INFO("## PID: %u - Proceso Creado. Tamano: %u. Tabla de Paginas inicializada y frames asignados.", pid, size);

    return 0;
}

process_info* process_manager_find_process(uint32_t pid) {
    process_info* found_proc = NULL;
    lock_process_list();
    for (int i = 0; i < list_size(global_process_list); i++) {
        process_info* proc = list_get(global_process_list, i);
        if (proc->pid == pid) {
            found_proc = proc;
            break;
        }
    }
    unlock_process_list();
    return found_proc;
}

int process_manager_delete_process(uint32_t pid) {
    int result = -1;
    lock_process_list();
    int found_index = -1;
    for (int i = 0; i < list_size(global_process_list); i++) {
        process_info* proc = list_get(global_process_list, i);
        if (proc->pid == pid) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        list_remove_and_destroy_element(global_process_list, found_index, destroy_process_info);
        LOG_INFO("## PID: %u - Proceso Finalizado y recursos liberados.", pid);
        result = 0;
    } else {
        LOG_ERROR("## PID: %u - Intento de finalizar proceso no existente.", pid);
    }
    unlock_process_list();
    return result;
}