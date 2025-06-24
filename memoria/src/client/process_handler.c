#include "process_handler.h"

extern t_memoria_config memoria_config;

// Helper to get the page table entry for a given virtual page number (VPN) at a specific level
static t_page_table_entry* get_pte_at_level(t_page_table* current_table, uint32_t virtual_address, int current_level, int total_levels, t_process_metrics* metrics) {
    uint32_t offset_bits_per_level = (uint32_t)log2(memoria_config.ENTRADAS_POR_TABLA);
    uint32_t page_offset_bits = (uint32_t)log2(memoria_config.TAM_PAGINA);

    uint32_t remaining_bits_for_vpn = (sizeof(uint32_t) * 8) - page_offset_bits;
    uint32_t vpn_prefix_length = remaining_bits_for_vpn - (total_levels - current_level) * offset_bits_per_level;
    uint32_t current_level_index = (virtual_address >> vpn_prefix_length) & ((1 << offset_bits_per_level) - 1);

    if (metrics) {
        metrics->page_table_access_count++;
    }
    usleep(memoria_config.RETARDO_MEMORIA * 1000);

    t_page_table_entry* entry = get_page_table_entry(current_table, current_level_index);
    if (entry == NULL) {
        LOG_ERROR("Error: Entrada de tabla de paginas no encontrada en nivel %d, indice %u.", current_level, current_level_index);
        return NULL;
    }

    if (current_level < total_levels) {
        if (entry->is_last_level) {
            LOG_ERROR("Error: Entrada de nivel %d marca como ultimo nivel pero se esperaban mas niveles.", current_level);
            return NULL;
        }
        t_page_table next_level_table_mock;
        next_level_table_mock.entries = entry->next_level;
        next_level_table_mock.num_entries = list_size(entry->next_level);

        return get_pte_at_level(&next_level_table_mock, virtual_address, current_level + 1, total_levels, metrics);
    } else {
        if (!entry->is_last_level) {
            LOG_ERROR("Error: Entrada de ultimo nivel %d no marca como ultimo nivel.", current_level);
            return NULL;
        }
        return entry;
    }
}

uint32_t translate_address(uint32_t pid, uint32_t virtual_address) {
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("## PID: %u - Error al traducir direccion: Proceso no encontrado.", pid);
        return (uint32_t)-1;
    }

    if (virtual_address >= proc->process_size) {
        LOG_ERROR("## PID: %u - Error al traducir direccion: Direccion virtual %u fuera del tamano del proceso %u.", pid, virtual_address, proc->process_size);
        return (uint32_t)-1;
    }

    uint32_t page_offset = virtual_address % memoria_config.TAM_PAGINA;

    t_page_table_entry* final_pte = get_pte_at_level(proc->page_table, virtual_address, 1, memoria_config.CANTIDAD_NIVELES, proc->metrics);

    if (final_pte == NULL) {
        LOG_ERROR("## PID: %u - Error al obtener PTE final para direccion virtual %u.", pid, virtual_address);
        return (uint32_t)-1;
    }

    uint32_t physical_address = final_pte->frame_number * memoria_config.TAM_PAGINA + page_offset;

    LOG_INFO("## PID: %u - Traduccion: Dir Virtual %u -> Marco %u -> Dir Fisica %u",
             pid, virtual_address, final_pte->frame_number, physical_address);

    return physical_address;
}

bool read_user_memory(uint32_t pid, uint32_t virtual_address, void* buffer, size_t size) {
    uint32_t physical_address = translate_address(pid, virtual_address);
    if (physical_address == (uint32_t)-1) {
        return false;
    }

    process_info* proc = process_manager_find_process(pid);
    if (proc) {
        proc->metrics->memory_read_count++;
    }

    return read_memory(physical_address, buffer, size);
}

bool write_user_memory(uint32_t pid, uint32_t virtual_address, const void* data, size_t size) {
    uint32_t physical_address = translate_address(pid, virtual_address);
    if (physical_address == (uint32_t)-1) {
        return false;
    }

    process_info* proc = process_manager_find_process(pid);
    if (proc) {
        proc->metrics->memory_write_count++;
    }

    return write_memory(physical_address, data, size);
}

// --- Server Request Handler Functions ---

void init_process_request_handler(int socket, t_package* package) {
    init_process_package_data* init_process_args = read_init_process_package(package);

    LOG_INFO("## PID: %u - Solicitud de creacion de Proceso Recibida.", init_process_args->pid);

    int result = process_manager_create_process(init_process_args->pid, init_process_args->size, init_process_args->pseudocode_path);

    destroy_init_process_package(init_process_args);

    send_confirmation_package(socket, result);
}

void get_instruction_request_handler(int socket, t_package* package) {
    fetch_package_data request = read_fetch_package(package);
    uint32_t pid = request.pid;
    uint32_t pc = request.pc;

    process_info* proc = process_manager_find_process(pid);

    if (proc != NULL) {
        proc->metrics->instruction_requests_count++;

        lock_process_instructions(); // Acquire lock for instructions access
        if (proc->instructions && pc < list_size(proc->instructions)) {
            char* instruction = list_get(proc->instructions, pc);
            if (instruction) {
                LOG_INFO("## PID: %u - Obtener Instruccion: %u - Instruccion: %s", pid, pc, instruction);
                send_instruction_package(socket, instruction);
            } else {
                LOG_ERROR("## PID: %u - PC %u la instruccion es NULL.", pid, pc);
                send_instruction_package(socket, "");
            }
        } else {
            LOG_ERROR("## PID: %u - PC %u fuera de limites (max: %d).", pid, pc,
                              proc->instructions ? list_size(proc->instructions) : 0);
            send_instruction_package(socket, "");
        }
        unlock_process_instructions(); // Release lock
    } else {
        LOG_ERROR("## PID: %u - Proceso no encontrado.", pid);
        send_instruction_package(socket, "");
    }
}

void delete_process_request_handler(int socket, t_package *package) {
    uint32_t pid_to_delete = read_kill_process_package(package);

    LOG_INFO("## PID: %u - Solicitud de Finalizacion de Proceso Recibida.", pid_to_delete);

    int result = process_manager_delete_process(pid_to_delete);

    if (result == 0) {
        LOG_INFO("## PID: %u - Proceso Finalizado y recursos liberados.", pid_to_delete);
        send_confirmation_package(socket, 0);
    } else {
        LOG_ERROR("## PID: %u - Intento de finalizar proceso no existente.", pid_to_delete);
        send_confirmation_package(socket, -1);
    }
}

void get_free_space_request_handler(int socket) {
    uint32_t free_frames_count = frame_get_free_count();
    uint32_t free_bytes = free_frames_count * memoria_config.TAM_PAGINA;

    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, free_bytes);

    t_package* package = create_package(0, buffer);
    send_package(socket, package);
    destroy_package(package);
    LOG_INFO("Enviando espacio libre: %u bytes (%u frames).", free_bytes, free_frames_count);
}