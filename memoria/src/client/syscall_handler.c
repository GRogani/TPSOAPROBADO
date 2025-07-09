#include "syscall_handler.h"
#include "../utils.h"
#include "kernel_space/process_manager.h"
#include "user_space/frame_manager.h"
#include "swap_space/swap_manager.h"
#include "semaphores.h"
#include <time.h>
#include "utils/DTPs/page_entry_package.h"

extern t_memoria_config memoria_config;

/**
 * @brief Navega por un nivel específico de la tabla de páginas
 * @param current_table Tabla de páginas actual
 * @param virtual_address Dirección virtual a traducir
 * @param current_level Nivel actual siendo procesado
 * @param total_levels Total de niveles en la jerarquía
 * @param metrics Métricas del proceso para actualizar
 * @return Puntero a la entrada de tabla de páginas final
 */
static t_page_table_entry* get_pte_at_level(t_page_table* current_table, uint32_t virtual_address, int current_level, int total_levels, t_process_metrics* metrics) {
    uint32_t offset_bits_per_level = (uint32_t)log2(memoria_config.ENTRADAS_POR_TABLA);
    uint32_t page_offset_bits = (uint32_t)log2(memoria_config.TAM_PAGINA);

    uint32_t remaining_bits_for_vpn = (sizeof(uint32_t) * 8) - page_offset_bits;
    uint32_t vpn_prefix_length = remaining_bits_for_vpn - (total_levels - current_level) * offset_bits_per_level;
    uint32_t current_level_index = (virtual_address >> vpn_prefix_length) & ((1 << offset_bits_per_level) - 1);

    if (metrics) {
        lock_process_metrics();
        metrics->page_table_access_count++;
        unlock_process_metrics();
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

/**
 * @brief Traduce una dirección virtual a física
 * @param pid ID del proceso
 * @param virtual_address Dirección virtual a traducir
 * @return Dirección física o (uint32_t)-1 si error
 */
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

    lock_page_table();
    t_page_table_entry* final_pte = get_pte_at_level(proc->page_table, virtual_address, 1, memoria_config.CANTIDAD_NIVELES, proc->metrics);
    unlock_page_table();

    if (final_pte == NULL) {
        LOG_ERROR("## PID: %u - Error al obtener PTE final para direccion virtual %u.", pid, virtual_address);
        return (uint32_t)-1;
    }

    uint32_t physical_address = final_pte->frame_number * memoria_config.TAM_PAGINA + page_offset;

    LOG_INFO("## PID: %u - Traduccion: Dir Virtual %u -> Marco %u -> Dir Fisica %u",
             pid, virtual_address, final_pte->frame_number, physical_address);

    return physical_address;
}

/**
 * @brief Lee datos de la memoria virtual de un proceso
 * @param pid ID del proceso
 * @param virtual_address Dirección virtual de inicio
 * @param buffer Buffer donde almacenar los datos
 * @param size Tamaño de datos a leer
 * @return true si éxito, false si error
 */
bool read_user_memory(uint32_t pid, uint32_t virtual_address, void* buffer, size_t size) {
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        return false;
    }

    uint32_t physical_address = translate_address(pid, virtual_address);
    if (physical_address == (uint32_t)-1) {
        return false;
    }

    lock_process_metrics();
    proc->metrics->memory_read_count++;
    unlock_process_metrics();

    return read_memory(physical_address, buffer, size);
}

/**
 * @brief Escribe datos en la memoria virtual de un proceso
 * @param pid ID del proceso
 * @param virtual_address Dirección virtual de inicio
 * @param data Datos a escribir
 * @param size Tamaño de datos a escribir
 * @return true si éxito, false si error
 */
bool write_user_memory(uint32_t pid, uint32_t virtual_address, const void* data, size_t size) {
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        return false;
    }

    uint32_t physical_address = translate_address(pid, virtual_address);
    if (physical_address == (uint32_t)-1) {
        return false;
    }

    lock_process_metrics();
    proc->metrics->memory_write_count++;
    unlock_process_metrics();

    return write_memory(physical_address, data, size);
}

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
        lock_process_metrics();
        proc->metrics->instruction_requests_count++;
        unlock_process_metrics();

        lock_process_instructions();
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
        unlock_process_instructions();
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

    t_package* package = create_package(GET_FREE_SPACE, buffer);
    send_package(socket, package);
    destroy_package(package);
    LOG_INFO("Enviando espacio libre: %u bytes (%u frames).", free_bytes, free_frames_count);
}

void write_memory_request_handler(int socket, t_package* package) {
    package->buffer->offset = 0;
    
    uint32_t physical_address = buffer_read_uint32(package->buffer);
    uint32_t data_size;
    char* data = buffer_read_string(package->buffer, &data_size);
    
    LOG_INFO("WRITE_MEMORY: Escritura en dirección física %u con datos: %s", physical_address, data);
    
    bool success = write_memory(physical_address, data, data_size);
    
    if (success) {
        LOG_INFO("WRITE_MEMORY: Escritura exitosa en dirección %u", physical_address);
        send_confirmation_package(socket, 0);
    } else {
        LOG_ERROR("WRITE_MEMORY: Error al escribir en dirección %u", physical_address);
        send_confirmation_package(socket, -1);
    }
    
    free(data);
}

void read_memory_request_handler(int socket, t_package* package) {
    package->buffer->offset = 0;
    
    uint32_t physical_address = buffer_read_uint32(package->buffer);
    uint32_t size = buffer_read_uint32(package->buffer);
    
    LOG_INFO("READ_MEMORY: Lectura de %u bytes desde dirección física %u", size, physical_address);
    
    char* buffer = malloc(size);
    if (buffer == NULL) {
        LOG_ERROR("READ_MEMORY: Error al asignar memoria para buffer de lectura");
        send_confirmation_package(socket, -1);
        return;
    }
    
    bool success = read_memory(physical_address, buffer, size);
    
    if (success) {
        LOG_INFO("READ_MEMORY: Lectura exitosa desde dirección %u", physical_address);
        
        t_buffer* response_buffer = buffer_create(size);
        buffer_add_string(response_buffer, size, buffer);
        t_package* response_package = create_package(READ_MEMORY, response_buffer);
        send_package(socket, response_package);
        destroy_package(response_package);
    } else {
        LOG_ERROR("READ_MEMORY: Error al leer desde dirección %u", physical_address);
        send_confirmation_package(socket, -1);
    }
    
    free(buffer);
}

void dump_memory_request_handler(int socket, t_package* package) {
    uint32_t pid = read_dump_memory_package(package);
    
    LOG_INFO("DUMP_MEMORY: Solicitud de dump para PID %u", pid);
    
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("DUMP_MEMORY: Proceso PID %u no encontrado", pid);
        send_confirmation_package(socket, -1);
        return;
    }
    
    lock_process_list();
    
    uint32_t process_size = proc->process_size;
    bool process_still_exists = process_manager_process_exists(pid);
    
    if (!process_still_exists) {
        LOG_ERROR("DUMP_MEMORY: Proceso PID %u fue eliminado durante el dump", pid);
        unlock_process_list();
        send_confirmation_package(socket, -1);
        return;
    }
    
    unlock_process_list();
    
    LOG_INFO("DUMP_MEMORY: Iniciando dump de memoria para PID %u", pid);
    
    if (memoria_config.DUMP_PATH != NULL) {
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", memoria_config.DUMP_PATH);
        system(mkdir_cmd);
    }
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    char dump_filename[512];
    if (memoria_config.DUMP_PATH != NULL) {
        snprintf(dump_filename, sizeof(dump_filename), "%s/%u-%04d%02d%02d_%02d%02d%02d.dmp", 
                 memoria_config.DUMP_PATH, pid, 
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec);
    } else {
        snprintf(dump_filename, sizeof(dump_filename), "%u-%04d%02d%02d_%02d%02d%02d.dmp", 
                 pid, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec);
    }
    
    FILE* dump_file = fopen(dump_filename, "wb");
    if (dump_file == NULL) {
        LOG_ERROR("DUMP_MEMORY: No se pudo crear archivo de dump para PID %u en %s", pid, dump_filename);
        send_confirmation_package(socket, -1);
        return;
    }
    
    bool dump_success = true;
    uint32_t total_bytes_dumped = 0;
    
    for (uint32_t page = 0; page < (process_size + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA; page++) {
        uint32_t virtual_address = page * memoria_config.TAM_PAGINA;
        uint32_t physical_address = translate_address(pid, virtual_address);
        
        if (physical_address != (uint32_t)-1) {
            char* page_buffer = malloc(memoria_config.TAM_PAGINA);
            if (page_buffer != NULL) {
                if (read_memory(physical_address, page_buffer, memoria_config.TAM_PAGINA)) {
                    fwrite(page_buffer, 1, memoria_config.TAM_PAGINA, dump_file);
                    total_bytes_dumped += memoria_config.TAM_PAGINA;
                } else {
                    LOG_ERROR("DUMP_MEMORY: Error leyendo página %u del proceso %u", page, pid);
                    dump_success = false;
                }
                free(page_buffer);
            } else {
                LOG_ERROR("DUMP_MEMORY: Error asignando memoria para dump de página %u", page);
                dump_success = false;
            }
        } else {
            LOG_WARNING("DUMP_MEMORY: Página %u del proceso %u no está en memoria (posiblemente en swap)", page, pid);
            char* zero_buffer = calloc(1, memoria_config.TAM_PAGINA);
            if (zero_buffer != NULL) {
                fwrite(zero_buffer, 1, memoria_config.TAM_PAGINA, dump_file);
                total_bytes_dumped += memoria_config.TAM_PAGINA;
                free(zero_buffer);
            }
        }
    }
    
    fclose(dump_file);
    
    if (dump_success) {
        LOG_INFO("DUMP_MEMORY: Dump completado para PID %u. Archivo: %s (%u bytes)", pid, dump_filename, total_bytes_dumped);
        send_confirmation_package(socket, 0);
    } else {
        LOG_ERROR("DUMP_MEMORY: Error durante dump para PID %u", pid);
        unlink(dump_filename);
        send_confirmation_package(socket, -1);
    }
}

void unsuspend_process_request_handler(int client_fd, t_package* package) {
    uint32_t pid = read_swap_package(package);
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("UNSUSPEND_PROCESS: Proceso PID %u no encontrado.", pid);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    if (!proc->is_suspended) {
        LOG_WARNING("UNSUSPEND_PROCESS: Proceso PID %u no está suspendido.", pid);
        send_confirmation_package(client_fd, 0);
        return;
    }
    
    if (proc->swap_pages_info == NULL) {
        LOG_ERROR("UNSUSPEND_PROCESS: Proceso PID %u no tiene información de swap.", pid);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    LOG_INFO("UNSUSPEND_PROCESS: Iniciando swap in para PID %u", pid);
    
    uint32_t pages_needed = list_size(proc->swap_pages_info);
    if (frame_get_free_count() < pages_needed) {
        LOG_ERROR("UNSUSPEND_PROCESS: No hay suficientes frames libres para PID %u (necesita %u, hay %u)", 
                  pid, pages_needed, frame_get_free_count());
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    t_list* new_frames = frame_allocate_frames(pages_needed);
    if (new_frames == NULL || list_size(new_frames) != pages_needed) {
        LOG_ERROR("UNSUSPEND_PROCESS: Error asignando frames para PID %u", pid);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    bool swap_in_success = true;
    uint32_t pages_restored = 0;
    
    lock_swap_file();
    
    for (uint32_t page = 0; page < pages_needed && swap_in_success; page++) {
        t_swap_page_info* swap_info = list_get(proc->swap_pages_info, page);
        uint32_t* frame_num_ptr = list_get(new_frames, page);
        
        if (swap_info != NULL && frame_num_ptr != NULL) {
            uint32_t physical_address = *frame_num_ptr * memoria_config.TAM_PAGINA;
            
            char* page_buffer = malloc(memoria_config.TAM_PAGINA);
            if (page_buffer != NULL) {
                if (swap_read_page(swap_info->swap_offset, page_buffer, memoria_config.TAM_PAGINA)) {
                    if (write_memory(physical_address, page_buffer, memoria_config.TAM_PAGINA)) {
                        pages_restored++;
                        LOG_DEBUG("UNSUSPEND_PROCESS: Página %u del PID %u restaurada desde swapfile.bin en frame %u", 
                                 page, pid, *frame_num_ptr);
                    } else {
                        LOG_ERROR("UNSUSPEND_PROCESS: Error escribiendo página %u del PID %u en memoria", page, pid);
                        swap_in_success = false;
                    }
                } else {
                    LOG_ERROR("UNSUSPEND_PROCESS: Error leyendo página %u del PID %u desde swapfile.bin", page, pid);
                    swap_in_success = false;
                }
                free(page_buffer);
            } else {
                LOG_ERROR("UNSUSPEND_PROCESS: Error asignando memoria para página %u", page);
                swap_in_success = false;
            }
        } else {
            LOG_ERROR("UNSUSPEND_PROCESS: Información de swap o frame no encontrada para página %u", page);
            swap_in_success = false;
        }
    }
    
    unlock_swap_file();
    
    if (swap_in_success) {
        bool page_table_update_success = update_process_page_table(proc, new_frames);
        if (!page_table_update_success) {
            LOG_ERROR("UNSUSPEND_PROCESS: Error actualizando tabla de páginas para PID %u", pid);
            frame_free_frames(new_frames);
            send_confirmation_package(client_fd, -1);
            return;
        }
        
        proc->is_suspended = false;
        proc->allocated_frames = new_frames;
        
        list_destroy_and_destroy_elements(proc->swap_pages_info, free);
        proc->swap_pages_info = NULL;
        
        lock_process_metrics();
        proc->metrics->swap_in_count++;
        unlock_process_metrics();
        
        LOG_INFO("UNSUSPEND_PROCESS: Swap in completado para PID %u. %u páginas restauradas desde swapfile.bin", pid, pages_restored);
        send_confirmation_package(client_fd, 0);
    } else {
        frame_free_frames(new_frames);
        
        LOG_ERROR("UNSUSPEND_PROCESS: Error durante swap in para PID %u", pid);
        send_confirmation_package(client_fd, -1);
    }
}

void swap_request_handler(int client_fd, t_package* package) {
    uint32_t pid = read_swap_package(package);
    LOG_INFO("SWAP: Iniciando swap out para PID %u", pid);
    
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("SWAP: Proceso PID %u no encontrado", pid);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    if (proc->is_suspended) {
        LOG_WARNING("SWAP: Proceso PID %u ya está suspendido", pid);
        send_confirmation_package(client_fd, 0);
        return;
    }
    
    bool swap_success = true;
    uint32_t pages_swapped = 0;
    
    lock_swap_file();
    
    uint32_t total_pages = (proc->process_size + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;
    
    t_list* swap_pages_info = swap_allocate_pages(pid, total_pages);
    if (swap_pages_info == NULL) {
        LOG_ERROR("SWAP: No se pudo asignar espacio en swapfile.bin para PID %u", pid);
        unlock_swap_file();
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    for (uint32_t page = 0; page < total_pages && swap_success; page++) {
        uint32_t virtual_address = page * memoria_config.TAM_PAGINA;
        uint32_t physical_address = translate_address(pid, virtual_address);
        
        if (physical_address != (uint32_t)-1) {
            char* page_buffer = malloc(memoria_config.TAM_PAGINA);
            if (page_buffer != NULL) {
                if (read_memory(physical_address, page_buffer, memoria_config.TAM_PAGINA)) {
                    t_swap_page_info* swap_info = list_get(swap_pages_info, page);
                    if (swap_info != NULL) {
                        swap_info->virtual_page_number = page;
                        
                        if (swap_write_page(swap_info->swap_offset, page_buffer, memoria_config.TAM_PAGINA)) {
                            pages_swapped++;
                            LOG_DEBUG("SWAP: Página %u del PID %u escrita en swapfile.bin en offset %u", 
                                     page, pid, swap_info->swap_offset);
                        } else {
                            LOG_ERROR("SWAP: Error escribiendo página %u del PID %u en swapfile.bin", page, pid);
                            swap_success = false;
                        }
                    } else {
                        LOG_ERROR("SWAP: Información de swap no encontrada para página %u", page);
                        swap_success = false;
                    }
                } else {
                    LOG_ERROR("SWAP: Error leyendo página %u del PID %u de memoria física", page, pid);
                    swap_success = false;
                }
                free(page_buffer);
            } else {
                LOG_ERROR("SWAP: Error asignando memoria para página %u", page);
                swap_success = false;
            }
        } else {
            LOG_WARNING("SWAP: Página %u del PID %u no está en memoria física", page, pid);
        }
    }
    
    unlock_swap_file();
    
    if (swap_success) {
        proc->is_suspended = true;
        
        if (proc->swap_pages_info != NULL) {
            list_destroy_and_destroy_elements(proc->swap_pages_info, free);
        }
        proc->swap_pages_info = swap_pages_info;
        
        if (proc->allocated_frames != NULL) {
            frame_free_frames(proc->allocated_frames);
            list_destroy(proc->allocated_frames);
            proc->allocated_frames = NULL;
        }
        
        lock_process_metrics();
        proc->metrics->swap_out_count++;
        unlock_process_metrics();
        
        LOG_INFO("SWAP: Swap out completado para PID %u. %u páginas swapeadas en swapfile.bin", pid, pages_swapped);
        send_confirmation_package(client_fd, 0);
    } else {
        lock_swap_file();
        swap_free_pages(swap_pages_info, memoria_config.TAM_PAGINA);
        unlock_swap_file();
        
        LOG_ERROR("SWAP: Error durante swap out para PID %u", pid);
        send_confirmation_package(client_fd, -1);
    }
}

void get_page_entry_request_handler(int socket, t_package* package) {
    package->buffer->offset = 0;
    
    page_entry_request_data request = read_page_entry_request_package(package);
    uint32_t pid = request.pid;
    uint32_t table_ptr = request.table_ptr;
    uint32_t entry_index = request.entry_index;
    
    LOG_INFO("GET_PAGE_ENTRY: PID: %u, Table PTR: %u, Entry Index: %u", pid, table_ptr, entry_index);
    
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("GET_PAGE_ENTRY: PID %u no encontrado", pid);
        send_page_entry_response_package(socket, 0xFFFFFFFF, false);
        return;
    }
    
    lock_process_metrics();
    proc->metrics->page_table_access_count++;
    unlock_process_metrics();
    
    lock_page_table();
    
    t_page_table* current_table = NULL;

    if (table_ptr == 0) {
        current_table = proc->page_table;
    } else {
        current_table = (t_page_table*)(uintptr_t)table_ptr;
    }
    
    if (current_table == NULL) {
        LOG_ERROR("GET_PAGE_ENTRY: Tabla no encontrada para PID %u, Table PTR %u", pid, table_ptr);
        unlock_page_table();
        send_page_entry_response_package(socket, 0xFFFFFFFF, 1);
        return;
    }

    t_page_table_entry* entry = get_page_table_entry(current_table, entry_index);
    if (entry == NULL) {
        LOG_ERROR("GET_PAGE_ENTRY: Entrada no encontrada en índice %u para PID %u", entry_index, pid);
        unlock_page_table();
        send_page_entry_response_package(socket, 0xFFFFFFFF, 1);
        return;
    }

    uint32_t return_value;
    bool is_last_level = entry->is_last_level;
    
    if (is_last_level) {
        return_value = entry->frame_number;
        LOG_INFO("GET_PAGE_ENTRY: Devolviendo frame_number %u para PID %u", return_value, pid);
    } else {
        return_value = (uint32_t)(uintptr_t)(entry->next_level);
        LOG_INFO("GET_PAGE_ENTRY: Devolviendo table_ptr %u para PID %u", return_value, pid);
    }
    
    unlock_page_table();

    send_page_entry_response_package(socket, return_value, is_last_level == true ? 0 : 1);
}