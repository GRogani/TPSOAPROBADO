#include "process_manager.h"

static t_list* global_process_list = NULL;

extern t_memoria_config memoria_config;

/**
 * @brief Asigna frames a las entradas de la tabla de páginas de forma recursiva
 * @param current_table Tabla de páginas actual
 * @param allocated_frames_for_process Lista de frames asignados al proceso
 * @param current_frame_index Índice del frame actual
 * @param total_levels Total de niveles en la jerarquía
 * @param current_level Nivel actual siendo procesado
 * @return true si éxito, false si error
 */
bool assign_frames_to_page_table(t_page_table* current_table, t_list* allocated_frames_for_process, int* current_frame_index, int total_levels, int current_level) {
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

/**
 * @brief Destruye la estructura process_info y libera todos sus recursos
 * @param proc_void_ptr Puntero a la estructura process_info
 */
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

    if (proc->swap_pages_info) {
        list_destroy_and_destroy_elements(proc->swap_pages_info, free);
    }

    if (proc->metrics) {
        free(proc->metrics);
    }

    free(proc);
}

/**
 * @brief Inicializa el process manager creando la lista global de procesos
 */
void process_manager_init() {
    global_process_list = list_create();
    if (global_process_list == NULL) {
        LOG_ERROR("Process Manager: Fallo al crear la lista global de procesos(PIDs).");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Process Manager: Inicializado.");
}

/**
 * @brief Destruye el process manager liberando todos los recursos
 */
void process_manager_destroy() {
    list_destroy_and_destroy_elements(global_process_list, destroy_process_info);
    LOG_INFO("Process Manager: Destruido.");
}

/**
 * @brief Carga las líneas de pseudocódigo desde un archivo
 * @param path Ruta del archivo de pseudocódigo
 * @return Lista con las líneas del script o NULL si error
 */
t_list* process_manager_load_script_lines(char* path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", memoria_config.PATH_INSTRUCCIONES, path);

    LOG_INFO("Intentando abrir archivo de pseudocodigo: %s", full_path);

    FILE *file = fopen(full_path, "r");
    if (!file) {
        LOG_ERROR("Script Load: Error al abrir archivo: %s", full_path);
        return NULL;
    }

    LOG_INFO("Archivo de pseudocodigo abierto correctamente: %s", full_path);

    t_list* list = list_create();
    char* line = NULL;
    size_t len = 0;
    ssize_t read_bytes;

    while ((read_bytes = getline(&line, &len, file)) != -1) {
        uint32_t line_len = strlen(line);
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

/**
 * @brief Crea un nuevo proceso con todos sus recursos
 * @param pid ID del proceso
 * @param size Tamaño en bytes del proceso
 * @param script_path Ruta al archivo de pseudocódigo
 * @return 0 si éxito, -1 si error
 */
int process_manager_create_process(uint32_t pid, uint32_t size, char* script_path) {
    process_info* proc = safe_malloc(sizeof(process_info));

    proc->pid = pid;
    proc->process_size = size;
    proc->is_suspended = false;
    proc->instructions = process_manager_load_script_lines(script_path);

    if (proc->instructions == NULL) {
        LOG_ERROR("## PID: %u - Error al cargar pseudocodigo desde: %s", pid, script_path);
        free(proc);
        return -1;
    }

    uint32_t pages_needed = (uint32_t)ceil((double)size / memoria_config.TAM_PAGINA);
    LOG_INFO("## PID: %u - Proceso requiere %d paginas (tamano: %d bytes, pagina: %d bytes).", pid, pages_needed, size, memoria_config.TAM_PAGINA);

    if (frame_get_free_count() < pages_needed) {
        LOG_ERROR("## PID: %u - No hay suficientes frames disponibles para crear el proceso. (Necesita %d, Libre %d)", pid, pages_needed, frame_get_free_count());
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

    proc->swap_pages_info = NULL;

    lock_process_list();
    list_add(global_process_list, proc);
    unlock_process_list();

    LOG_INFO("## PID: %u - Proceso Creado. Tamano: %u. Tabla de Paginas inicializada y frames asignados.", pid, size);

    return 0;
}

/**
 * @brief Busca un proceso por PID en la lista global
 * @param pid ID del proceso a buscar
 * @return Puntero al proceso o NULL si no existe
 */
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

/**
 * @brief Actualiza la tabla de páginas de un proceso con nuevos frames
 * @param proc Proceso a actualizar
 * @param new_frames Lista con los nuevos frames
 * @return true si éxito, false si error
 */
bool update_process_page_table(process_info* proc, t_list* new_frames) {
    if (proc == NULL || new_frames == NULL) {
        LOG_ERROR("Error: Proceso o lista de frames es NULL en update_process_page_table.");
        return false;
    }

    lock_page_table();
    
    bool reset_success = reset_page_table_frames(proc->page_table, memoria_config.CANTIDAD_NIVELES, 1);
    if (!reset_success) {
        LOG_ERROR("Error: No se pudo resetear la tabla de páginas para PID %u.", proc->pid);
        unlock_page_table();
        return false;
    }

    int current_frame_idx = 0;
    bool assign_success = assign_frames_to_page_table(proc->page_table, new_frames, &current_frame_idx, memoria_config.CANTIDAD_NIVELES, 1);
    
    unlock_page_table();
    
    if (!assign_success) {
        LOG_ERROR("Error: No se pudo asignar nuevos frames a la tabla de páginas para PID %u.", proc->pid);
        return false;
    }

    LOG_INFO("## PID: %u - Tabla de páginas actualizada con %d nuevos frames.", proc->pid, list_size(new_frames));
    return true;
}

/**
 * @brief Elimina un proceso y libera todos sus recursos
 * @param pid ID del proceso a eliminar
 * @return 0 si éxito, -1 si el proceso no existe
 */
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

/**
 * @brief Verifica si un proceso existe en la lista global
 * @param pid ID del proceso a verificar
 * @return true si existe, false en caso contrario
 */
bool process_manager_process_exists(uint32_t pid) {
    bool exists = false;
    lock_process_list();
    for (int i = 0; i < list_size(global_process_list); i++) {
        process_info* proc = list_get(global_process_list, i);
        if (proc->pid == pid) {
            exists = true;
            break;
        }
    }
    unlock_process_list();
    return exists;
}