#include "syscall_handler.h"
#include "../utils.h"
#include "kernel_space/process_manager.h"
#include "user_space/frame_manager.h"
#include "swap_space/swap_manager.h"
#include "semaphores.h"
#include <time.h>
#include "utils/DTPs/page_entry_package.h"
#include "kernel_space/page_table.h"

extern t_memoria_config memoria_config;

/**
 * @brief Obtiene la tabla de páginas en un nivel específico de la jerarquía
 * @param root_table Tabla raíz del proceso
 * @param target_level Nivel objetivo a obtener
 * @param total_levels Total de niveles en la jerarquía
 * @return Puntero a la tabla en el nivel especificado, o NULL si no se encuentra
 */
static t_page_table* get_table_at_level(t_page_table* root_table, uint32_t target_level, int total_levels) {
    if (root_table == NULL || target_level < 1 || target_level > total_levels) {
        return NULL;
    }
    
    // Si queremos el nivel 1 (raíz), lo devolvemos directamente
    if (target_level == 1) {
        return root_table;
    }
    
    // Para niveles más profundos, necesitamos navegar por la jerarquía
    t_page_table* current_table = root_table;
    
    for (uint32_t level = 1; level < target_level; level++) {
        // Buscar la primera entrada no-nula en el nivel actual
        bool found_next_level = false;
        
        for (int i = 0; i < current_table->num_entries; i++) {
            t_page_table_entry* entry = get_page_table_entry(current_table, i);
            if (entry != NULL && !entry->is_last_level && entry->next_table != NULL) {
                current_table = entry->next_table;
                found_next_level = true;
                break;
            }
        }
        
        if (!found_next_level) {
            LOG_ERROR("get_table_at_level: No se pudo encontrar tabla en nivel %u", level + 1);
            return NULL;
        }
    }
    
    return current_table;
}

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
    
    // Calculamos el número de página virtual completo
    uint32_t virtual_page_number = virtual_address / memoria_config.TAM_PAGINA;
    
    // Extraer bits correspondientes al índice del nivel actual
    // Para nivel 1, tomamos los bits más significativos, para nivel 2 los siguientes, etc.
    uint32_t bits_per_index = offset_bits_per_level;
    uint32_t shift_amount = page_offset_bits + bits_per_index * (total_levels - current_level);
    uint32_t current_level_index = (virtual_address >> shift_amount) & ((1 << bits_per_index) - 1);
    
    LOG_DEBUG("TRANSLATE_LEVEL: VA %u, VPN %u, Nivel %d, Índice %u, Desplaz %u", 
             virtual_address, virtual_page_number, current_level, current_level_index, shift_amount);

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
        if (entry->next_table != NULL) {
            return get_pte_at_level(entry->next_table, virtual_address, current_level + 1, total_levels, metrics);
        } else {
            LOG_ERROR("Error: Entrada de nivel %d no tiene tabla siguiente.", current_level);
            return NULL;
        }
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

    // Calcular número de página virtual y offset
    uint32_t virtual_page_number = virtual_address / memoria_config.TAM_PAGINA;
    uint32_t page_offset = virtual_address % memoria_config.TAM_PAGINA;
    
    LOG_DEBUG("TRANSLATE: PID %u, Dir. Virtual %u -> Página %u, Offset %u", 
             pid, virtual_address, virtual_page_number, page_offset);
    
    // Si el proceso está suspendido, es posible que la página esté en swap
    if (proc->is_suspended && proc->swap_pages_info != NULL) {
        bool page_in_swap = false;
        
        // Verificar si esta página está en swap
        for (int i = 0; i < list_size(proc->swap_pages_info); i++) {
            t_swap_page_info* swap_info = list_get(proc->swap_pages_info, i);
            if (swap_info->virtual_page_number == virtual_page_number) {
                LOG_DEBUG("TRANSLATE: PID %u, Página %u está en swap (offset %u)", 
                         pid, virtual_page_number, swap_info->swap_offset);
                page_in_swap = true;
                break;
            }
        }
        
        if (page_in_swap) {
            LOG_DEBUG("TRANSLATE: Página %u del proceso %u está en swap, no en memoria", virtual_page_number, pid);
            return (uint32_t)-1;
        }
    }

    lock_page_table();
    t_page_table_entry* final_pte = get_pte_at_level(proc->page_table, virtual_address, 1, memoria_config.CANTIDAD_NIVELES, proc->metrics);
    unlock_page_table();

    if (final_pte == NULL) {
        LOG_ERROR("## PID: %u - Error al obtener PTE final para direccion virtual %u.", pid, virtual_address);
        return (uint32_t)-1;
    }

    if (final_pte->frame_number == INVALID_FRAME_NUMBER) {
        LOG_DEBUG("## PID: %u - Frame number invalido para direccion virtual %u (posiblemente en swap).", pid, virtual_address);
        return (uint32_t)-1;
    }

    uint32_t physical_address = final_pte->frame_number * memoria_config.TAM_PAGINA + page_offset;
    LOG_DEBUG("TRANSLATE: PID %u, Dir. Virtual %u -> Dir. Física %u (Frame %u)", 
             pid, virtual_address, physical_address, final_pte->frame_number);

    LOG_OBLIGATORIO("## PID: %u - Traduccion: Dir Virtual %u -> Marco %u -> Dir Fisica %u",
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

    LOG_OBLIGATORIO("## PID: %u - Lectura de memoria virtual en dirección %u, tamaño %zu", pid, virtual_address, size);
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

    LOG_OBLIGATORIO("## PID: %u - Escritura de memoria virtual en dirección %u, tamaño %zu", pid, virtual_address, size);
    return write_memory(physical_address, data, size);
}

void init_process_request_handler(int socket, t_package* package) {
    init_process_package_data* init_process_args = read_init_process_package(package);

    LOG_OBLIGATORIO("## PID: %u - Solicitud de creacion de Proceso Recibida.", init_process_args->pid);

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
                LOG_OBLIGATORIO("## PID: %u - Obtener instrucción: %u - Instrucción: %s", pid, pc, instruction);
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
        LOG_OBLIGATORIO("## PID: %u - Proceso Finalizado y recursos liberados.", pid_to_delete);
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
    
    // Imprimir datos para depuración
    char debug_data[17] = {0};
    memcpy(debug_data, data, data_size > 16 ? 16 : data_size);
    for (int i = 0; i < 16 && i < data_size; i++) {
        if (!isprint(debug_data[i])) debug_data[i] = '.';
    }
    
    LOG_DEBUG("WRITE_MEMORY: Escribiendo en dirección física %u con datos (primeros 16 bytes): '%s'", 
             physical_address, debug_data);
    
    // Calcular frame number para referencia
    uint32_t frame_number = physical_address / memoria_config.TAM_PAGINA;
    uint32_t offset = physical_address % memoria_config.TAM_PAGINA;
    LOG_DEBUG("WRITE_MEMORY: Dirección %u corresponde a Frame %u, Offset %u", 
             physical_address, frame_number, offset);
    
    bool success = write_memory(physical_address, data, data_size);
    
    if (success) {
        // Verificación: leer los datos escritos para comprobar
        char* verify_buffer = malloc(data_size);
        if (verify_buffer && read_memory(physical_address, verify_buffer, data_size)) {
            char verify_debug[17] = {0};
            memcpy(verify_debug, verify_buffer, data_size > 16 ? 16 : data_size);
            for (int i = 0; i < 16 && i < data_size; i++) {
                if (!isprint(verify_debug[i])) verify_debug[i] = '.';
            }
            
            LOG_DEBUG("WRITE_MEMORY: Verificación - leídos (primeros 16 bytes): '%s'", verify_debug);
            if (memcmp(data, verify_buffer, data_size) == 0) {
                LOG_DEBUG("WRITE_MEMORY: Verificación exitosa - datos coinciden");
            } else {
                LOG_WARNING("WRITE_MEMORY: Verificación falló - datos no coinciden");
            }
            free(verify_buffer);
        }
        
        LOG_OBLIGATORIO("WRITE_MEMORY: Escritura exitosa en dirección física %u, frame %u", 
                     physical_address, frame_number);
        send_confirmation_package(socket, 0);
    } else {
        LOG_ERROR("WRITE_MEMORY: Error al escribir en dirección física %u", physical_address);
        send_confirmation_package(socket, -1);
    }
    
    free(data);
}

void read_memory_request_handler(int socket, t_package* package) {
    package->buffer->offset = 0;
    
    uint32_t physical_address = buffer_read_uint32(package->buffer);
    uint32_t size = buffer_read_uint32(package->buffer);
    
    //LOG_INFO("READ_MEMORY: Lectura de %u bytes desde dirección física %u", size, physical_address);
    
    char* buffer = malloc(size);
    if (buffer == NULL) {
        LOG_ERROR("READ_MEMORY: Error al asignar memoria para buffer de lectura");
        send_confirmation_package(socket, -1);
        return;
    }
    
    bool success = read_memory(physical_address, buffer, size);
    
    if (success) {
        LOG_INFO("READ_MEMORY: Lectura exitosa desde dirección %u", physical_address);
        send_memory_read_response(socket, buffer, size);
    } else {
        LOG_ERROR("READ_MEMORY: Error al leer desde dirección %u", physical_address);
        send_confirmation_package(socket, -1);
    }
    
    free(buffer);
}

void dump_memory_request_handler(int socket, t_package* package) {
    uint32_t pid = read_dump_memory_package(package);
    LOG_OBLIGATORIO("## PID: %u - Memory Dump solicitado", pid);
    
    process_info* proc = process_manager_find_process(pid);
    if (proc == NULL) {
        LOG_ERROR("DUMP_MEMORY: Proceso PID %u no encontrado", pid);
        send_confirmation_package(socket, -1);
        return;
    }
    
    uint32_t process_size = proc->process_size;
    bool process_still_exists = process_manager_process_exists(pid);
    
    if (!process_still_exists) {
        LOG_ERROR("DUMP_MEMORY: Proceso PID %u fue eliminado durante el dump", pid);
        send_confirmation_package(socket, -1);
        return;
    }
    
    LOG_OBLIGATORIO("DUMP_MEMORY: Iniciando dump de memoria para PID %u (tamaño: %u bytes)", pid, process_size);
    
    // Verificar si el proceso tiene información de swap
    bool has_swap_info = (proc->swap_pages_info != NULL && !list_is_empty(proc->swap_pages_info));
    if (has_swap_info) {
        LOG_OBLIGATORIO("DUMP_MEMORY: Proceso PID %u tiene %d páginas en swap", 
                pid, proc->swap_pages_info ? list_size(proc->swap_pages_info) : 0);
    } else {
        LOG_DEBUG("DUMP_MEMORY: Proceso PID %u no tiene páginas en swap", pid);
    }
    
    // Crear directorio de dump si es necesario
    if (memoria_config.DUMP_PATH != NULL) {
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", memoria_config.DUMP_PATH);
        system(mkdir_cmd);
    }
    
    // Generar nombre de archivo con timestamp
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
    
    LOG_DEBUG("DUMP_MEMORY: Archivo de dump creado: %s", dump_filename);
    
    // Proceso en memoria y sus páginas en swap
    void* process_memory = malloc(process_size);
    if (process_memory == NULL) {
        LOG_ERROR("DUMP_MEMORY: No se pudo asignar memoria para el proceso %u", pid);
        fclose(dump_file);
        send_confirmation_package(socket, -1);
        return;
    }
    
    memset(process_memory, 0, process_size); // Inicializar a ceros
    
    // Calcular número total de páginas
    uint32_t total_pages = (process_size + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;
    LOG_DEBUG("DUMP_MEMORY: Proceso PID %u tiene %u páginas en total", pid, total_pages);
    
    // Crear un mapa para registrar las páginas ya procesadas
    bool* page_processed = calloc(total_pages, sizeof(bool));
    if (!page_processed) {
        LOG_ERROR("DUMP_MEMORY: Error asignando memoria para control de páginas procesadas");
        free(process_memory);
        fclose(dump_file);
        send_confirmation_package(socket, -1);
        return;
    }
    
    bool dump_success = true;
    uint32_t total_bytes_dumped = 0;
    uint32_t pages_in_memory = 0;
    
    // PASO 1: Primero intentar cargar las páginas que están en memoria
    for (uint32_t page = 0; page < total_pages; page++) {
        // Evitar procesar la misma página virtual más de una vez
        if (page_processed[page]) {
            LOG_DEBUG("DUMP_MEMORY: Página %u ya fue procesada, saltando", page);
            continue;
        }
        
        uint32_t virtual_address = page * memoria_config.TAM_PAGINA;
        uint32_t physical_address = translate_address(pid, virtual_address);
        
        if (physical_address != (uint32_t)-1) {
            // La página está en memoria física
            pages_in_memory++;
            char* page_buffer = process_memory + virtual_address;
            
            if (read_memory(physical_address, page_buffer, memoria_config.TAM_PAGINA)) {
                LOG_DEBUG("DUMP_MEMORY: Leída página %u del proceso %u desde memoria física", page, pid);
                
                // Marcar esta página como procesada
                page_processed[page] = true;
                
                // Debug: Mostrar primeros bytes
                char debug_data[17] = {0};
                memcpy(debug_data, page_buffer, 16);
                for (int i = 0; i < 16; i++) {
                    if (!isprint(debug_data[i])) debug_data[i] = '.';
                }
                LOG_DEBUG("DUMP_MEMORY: Datos de página %u en memoria: '%s'", page, debug_data);
            } else {
                LOG_ERROR("DUMP_MEMORY: Error leyendo página %u del proceso %u desde memoria física", page, pid);
                dump_success = false;
            }
        }
    }
    
    LOG_DEBUG("DUMP_MEMORY: %u páginas encontradas en memoria física", pages_in_memory);
    
    // PASO 2: Si el proceso está suspendido y tiene páginas en swap, cargarlas
    if (has_swap_info && proc->is_suspended) {
        LOG_OBLIGATORIO("DUMP_MEMORY: Cargando páginas de swap para proceso %u", pid);
        
        // Usar una copia temporal de las páginas de swap para evitar modificar la original
        t_list* swap_pages_to_read = list_create();
        if (!swap_pages_to_read) {
            LOG_ERROR("DUMP_MEMORY: Error creando lista para páginas de swap");
            free(page_processed);
            free(process_memory);
            fclose(dump_file);
            send_confirmation_package(socket, -1);
            return;
        }
        
        // Solo agregar a la lista de lectura aquellas páginas que no se han procesado aún
        for (int i = 0; i < list_size(proc->swap_pages_info); i++) {
            t_swap_page_info* page_info = list_get(proc->swap_pages_info, i);
            uint32_t virtual_page_num = page_info->virtual_page_number;
            
            if (virtual_page_num < total_pages && !page_processed[virtual_page_num]) {
                // Crear una copia de la información de la página
                t_swap_page_info* page_info_copy = malloc(sizeof(t_swap_page_info));
                if (page_info_copy) {
                    *page_info_copy = *page_info;  // Copiar la estructura
                    list_add(swap_pages_to_read, page_info_copy);
                    LOG_DEBUG("DUMP_MEMORY: Agregando página virtual %u para leer desde swap", virtual_page_num);
                }
            } else {
                LOG_DEBUG("DUMP_MEMORY: Página virtual %u ya está en memoria o fuera de rango, no se leerá de swap",
                         virtual_page_num);
            }
        }
        
        // Solo leer de swap si hay páginas pendientes
        if (list_size(swap_pages_to_read) > 0) {
            LOG_OBLIGATORIO("DUMP_MEMORY: Leyendo %d páginas desde swap", list_size(swap_pages_to_read));
            
            if (!swap_read_pages(swap_pages_to_read, process_memory, memoria_config.TAM_PAGINA)) {
                LOG_ERROR("DUMP_MEMORY: Error leyendo páginas de swap para proceso %u", pid);
                dump_success = false;
            } else {
                LOG_OBLIGATORIO("DUMP_MEMORY: %d páginas cargadas exitosamente desde swap", 
                              list_size(swap_pages_to_read));
                
                // Marcar estas páginas como procesadas
                for (int i = 0; i < list_size(swap_pages_to_read); i++) {
                    t_swap_page_info* page_info = list_get(swap_pages_to_read, i);
                    uint32_t virtual_page_num = page_info->virtual_page_number;
                    
                    if (virtual_page_num < total_pages) {
                        page_processed[virtual_page_num] = true;
                        
                        // Debug: Mostrar datos leídos
                        uint32_t vaddr = virtual_page_num * memoria_config.TAM_PAGINA;
                        char* page_buffer = process_memory + vaddr;
                        
                        char debug_data[17] = {0};
                        memcpy(debug_data, page_buffer, 16);
                        for (int j = 0; j < 16; j++) {
                            if (!isprint(debug_data[j])) debug_data[j] = '.';
                        }
                        LOG_DEBUG("DUMP_MEMORY: Página %u leída desde swap: '%s'", 
                                virtual_page_num, debug_data);
                    }
                }
            }
        }
        
        // Liberar la lista temporal
        list_destroy_and_destroy_elements(swap_pages_to_read, free);
    }
    
    // PASO 3: Verificar si hay páginas que no se pudieron cargar
    int unprocessed_pages = 0;
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!page_processed[i]) {
            unprocessed_pages++;
            LOG_WARNING("DUMP_MEMORY: Página virtual %u no se pudo cargar ni de memoria ni de swap", i);
        }
    }
    
    if (unprocessed_pages > 0) {
        LOG_WARNING("DUMP_MEMORY: %d páginas no pudieron ser cargadas", unprocessed_pages);
    }
    
    // Escribir el proceso completo al archivo
    if (dump_success) {
        size_t written = fwrite(process_memory, 1, process_size, dump_file);
        if (written != process_size) {
            LOG_ERROR("DUMP_MEMORY: Error escribiendo al archivo de dump (escritos %zu de %u bytes)",
                    written, process_size);
            dump_success = false;
        } else {
            total_bytes_dumped = written;
        }
    }
    
    free(process_memory);
    free(page_processed);
    fclose(dump_file);
    
    if (dump_success) {
        LOG_OBLIGATORIO("DUMP_MEMORY: Dump completado para PID %u. Archivo: %s (%u bytes)", 
                pid, dump_filename, total_bytes_dumped);
        send_confirmation_package(socket, 0);
    } else {
        LOG_ERROR("DUMP_MEMORY: Error durante dump para PID %u", pid);
        unlink(dump_filename);
        send_confirmation_package(socket, -1);
    }
}

void unsuspend_process_request_handler(int client_fd, t_package* package) {
    usleep(memoria_config.RETARDO_SWAP * 1000);  // Aplicar retardo de swap

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
    
    if (proc->swap_pages_info == NULL || list_is_empty(proc->swap_pages_info)) {
        LOG_ERROR("UNSUSPEND_PROCESS: Proceso PID %u no tiene información de swap válida.", pid);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    LOG_OBLIGATORIO("UNSUSPEND_PROCESS: Iniciando swap in para PID %u", pid);
    
    // Verificar que todas las páginas virtuales tengan su asignación en swap
    uint32_t total_pages = (proc->process_size + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;
    uint32_t pages_in_swap = list_size(proc->swap_pages_info);
    
    LOG_OBLIGATORIO("UNSUSPEND_PROCESS: PID %u tiene %u páginas en swap, necesita %u páginas", 
                   pid, pages_in_swap, total_pages);
    
    if (pages_in_swap != total_pages) {
        LOG_WARNING("UNSUSPEND_PROCESS: Número de páginas en swap (%u) no coincide con el tamaño del proceso (%u páginas)",
                   pages_in_swap, total_pages);
    }
    
    uint32_t pages_needed = pages_in_swap;
    
    // Verificar disponibilidad de frames
    if (frame_get_free_count() < pages_needed) {
        LOG_ERROR("UNSUSPEND_PROCESS: No hay suficientes frames libres para PID %u (necesita %u, hay %u)", 
                  pid, pages_needed, frame_get_free_count());
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    // Asignar frames físicos
    t_list* new_frames = frame_allocate_frames(pages_needed);
    if (new_frames == NULL || list_size(new_frames) != pages_needed) {
        LOG_ERROR("UNSUSPEND_PROCESS: Error asignando frames para PID %u", pid);
        if (new_frames) {
            frame_free_frames(new_frames);
            list_destroy(new_frames);
        }
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    LOG_OBLIGATORIO("UNSUSPEND_PROCESS: Asignados %u frames para PID %u", list_size(new_frames), pid);
    
    // Crear buffer temporal para todas las páginas del proceso
    char* process_memory = calloc(pages_needed, memoria_config.TAM_PAGINA);
    if (process_memory == NULL) {
        LOG_ERROR("UNSUSPEND_PROCESS: Error asignando memoria temporal para PID %u", pid);
        frame_free_frames(new_frames);
        list_destroy(new_frames);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    // Inicializar buffer con ceros
    memset(process_memory, 0, pages_needed * memoria_config.TAM_PAGINA);
    
    // Leer todas las páginas desde el archivo de swap
    if (swap_read_pages(proc->swap_pages_info, process_memory, memoria_config.TAM_PAGINA)) {
        LOG_OBLIGATORIO("UNSUSPEND_PROCESS: Páginas leídas correctamente de swap para PID %u", pid);
        
        // Verificar contenido leído para debug
        for (uint32_t page = 0; page < pages_needed; page++) {
            void* page_content = process_memory + (page * memoria_config.TAM_PAGINA);
            char debug_data[17] = {0};
            memcpy(debug_data, page_content, 16);
            for (int j = 0; j < 16; j++) {
                if (!isprint(debug_data[j])) debug_data[j] = '.';
            }
            LOG_DEBUG("UNSUSPEND_PROCESS: Página %u leída - primeros bytes: '%s'", page, debug_data);
        }
        
        // Escribir páginas a los nuevos frames físicos
        bool write_success = true;
        
        for (uint32_t i = 0; i < pages_needed && write_success; i++) {
            // Obtener la información de la página virtual
            t_swap_page_info* page_info = list_get(proc->swap_pages_info, i);
            uint32_t virtual_page_num = page_info->virtual_page_number;
            uint32_t* frame_num_ptr = list_get(new_frames, i);
            
            if (frame_num_ptr != NULL) {
                uint32_t physical_address = *frame_num_ptr * memoria_config.TAM_PAGINA;
                void* page_content = process_memory + (i * memoria_config.TAM_PAGINA);
                
                LOG_OBLIGATORIO("UNSUSPEND_PROCESS: Escribiendo página virtual %u en frame físico %u",
                               virtual_page_num, *frame_num_ptr);
                
                if (!write_memory(physical_address, page_content, memoria_config.TAM_PAGINA)) {
                    LOG_ERROR("UNSUSPEND_PROCESS: Error escribiendo página virtual %u del PID %u en memoria física",
                             virtual_page_num, pid);
                    write_success = false;
                }
            } else {
                LOG_ERROR("UNSUSPEND_PROCESS: Frame no encontrado para página %u", i);
                write_success = false;
            }
        }
        
        if (write_success) {
            bool page_table_update_success = update_process_page_table(proc, new_frames);
            if (!page_table_update_success) {
                LOG_ERROR("UNSUSPEND_PROCESS: Error actualizando tabla de páginas para PID %u", pid);
                frame_free_frames(new_frames);
                list_destroy(new_frames);
                free(process_memory);
                send_confirmation_package(client_fd, -1);
                return;
            }
            
            // Actualizar estado del proceso
            proc->is_suspended = false;
            proc->allocated_frames = new_frames;
            
            // Conservar la referencia al swap_pages_info antes de liberarlo
            t_list* old_swap_pages_info = proc->swap_pages_info;
            proc->swap_pages_info = NULL;
            
            // Actualizar métricas
            lock_process_metrics();
            proc->metrics->swap_in_count++;
            unlock_process_metrics();
            
            // Liberar páginas del swap
            LOG_OBLIGATORIO("## UNSUSPEND_PROCESS - PID: %u - Liberando páginas de swap", pid);
            swap_free_pages(pid);
            
            // Ahora que swap_free_pages terminó, destruimos la lista
            if (old_swap_pages_info != NULL) {
                list_destroy_and_destroy_elements(old_swap_pages_info, free);
            }
            
            LOG_OBLIGATORIO("## UNSUSPEND_PROCESS - PID: %u - Swap in completado (%u páginas restauradas)", pid, pages_needed);
            send_confirmation_package(client_fd, 0);
        } else {
            frame_free_frames(new_frames);
            list_destroy(new_frames);
            LOG_ERROR("UNSUSPEND_PROCESS: Error escribiendo páginas a memoria para PID %u", pid);
            send_confirmation_package(client_fd, -1);
        }
    } else {
        frame_free_frames(new_frames);
        list_destroy(new_frames);
        LOG_ERROR("UNSUSPEND_PROCESS: Error leyendo páginas desde swap para PID %u", pid);
        send_confirmation_package(client_fd, -1);
    }
    
    free(process_memory);
}

void swap_request_handler(int client_fd, t_package* package) {
    usleep(memoria_config.RETARDO_SWAP * 1000);
    
    uint32_t pid = read_swap_package(package);
    LOG_OBLIGATORIO("SWAP: Iniciando swap out para PID %u", pid);
    
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
    
    uint32_t total_pages = (proc->process_size + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;
    LOG_OBLIGATORIO("SWAP: PID %u necesita %u páginas en swap", pid, total_pages);
    
    // Si ya hay información de swap, liberarla primero
    if (proc->swap_pages_info != NULL) {
        LOG_WARNING("SWAP: PID %u ya tiene información de swap, liberando primero", pid);
        swap_free_pages(pid);
        if (proc->swap_pages_info) {
            list_destroy_and_destroy_elements(proc->swap_pages_info, free);
            proc->swap_pages_info = NULL;
        }
    }
    
    // Asignar espacio en swap para todas las páginas del proceso
    t_list* swap_pages_info = swap_allocate_pages(pid, total_pages);
    if (swap_pages_info == NULL) {
        LOG_ERROR("SWAP: No se pudo asignar espacio en swapfile.bin para PID %u", pid);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    // Verificar que tenemos todas las páginas necesarias asignadas
    if (list_size(swap_pages_info) != total_pages) {
        LOG_ERROR("SWAP: Se asignaron %d páginas en swap pero se necesitaban %u para PID %u", 
                  list_size(swap_pages_info), total_pages, pid);
        list_destroy_and_destroy_elements(swap_pages_info, free);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    // Crear buffer temporal para todas las páginas del proceso
    char* process_memory = calloc(total_pages, memoria_config.TAM_PAGINA);
    if (process_memory == NULL) {
        LOG_ERROR("SWAP: Error asignando memoria temporal para PID %u", pid);
        list_destroy_and_destroy_elements(swap_pages_info, free);
        send_confirmation_package(client_fd, -1);
        return;
    }
    
    // Inicializar el buffer de memoria con ceros
    memset(process_memory, 0, total_pages * memoria_config.TAM_PAGINA);
    
    // Leer todas las páginas del proceso desde memoria física
    bool read_success = true;
    int pages_read = 0;
    
    for (uint32_t page = 0; page < total_pages && read_success; page++) {
        uint32_t virtual_address = page * memoria_config.TAM_PAGINA;
        uint32_t physical_address = translate_address(pid, virtual_address);
        
        if (physical_address != (uint32_t)-1) {
            void* page_dest = process_memory + (page * memoria_config.TAM_PAGINA);
            if (!read_memory(physical_address, page_dest, memoria_config.TAM_PAGINA)) {
                LOG_ERROR("SWAP: Error leyendo página %u del PID %u de memoria física", page, pid);
                read_success = false;
            } else {
                pages_read++;
                
                // Log para depuración
                char debug_data[17] = {0};
                memcpy(debug_data, page_dest, 16);
                for (int j = 0; j < 16; j++) {
                    if (!isprint(debug_data[j])) debug_data[j] = '.';
                }
                LOG_OBLIGATORIO("SWAP: Leída página %u - primeros bytes: '%s'", page, debug_data);
            }
        } else {
            LOG_INFO("SWAP: Página %u del PID %u no está en memoria física", page, pid);
        }
    }
    
    LOG_OBLIGATORIO("SWAP: Leídas %d páginas de memoria para PID %u", pages_read, pid);
    
    if (read_success) {
        // Verificar que cada página virtual tenga su entrada en swap_pages_info
        bool pages_valid = true;
        bool* page_assigned = calloc(total_pages, sizeof(bool));
        
        if (!page_assigned) {
            LOG_ERROR("SWAP: Error asignando memoria para verificación");
            free(process_memory);
            list_destroy_and_destroy_elements(swap_pages_info, free);
            send_confirmation_package(client_fd, -1);
            return;
        }
        
        for (int i = 0; i < list_size(swap_pages_info); i++) {
            t_swap_page_info* page_info = list_get(swap_pages_info, i);
            if (page_info->virtual_page_number >= total_pages) {
                LOG_ERROR("SWAP: Página virtual %u fuera de rango para PID %u", 
                          page_info->virtual_page_number, pid);
                pages_valid = false;
                break;
            }
            
            if (page_assigned[page_info->virtual_page_number]) {
                LOG_ERROR("SWAP: Página virtual %u asignada múltiples veces para PID %u", 
                          page_info->virtual_page_number, pid);
                pages_valid = false;
                break;
            }
            
            page_assigned[page_info->virtual_page_number] = true;
        }
        
        // Verificar que todas las páginas tengan asignación
        for (uint32_t i = 0; i < total_pages; i++) {
            if (!page_assigned[i]) {
                LOG_ERROR("SWAP: Página virtual %u sin asignación en swap para PID %u", i, pid);
                pages_valid = false;
                break;
            }
        }
        
        free(page_assigned);
        
        if (!pages_valid) {
            LOG_ERROR("SWAP: Error en asignación de páginas virtuales para PID %u", pid);
            free(process_memory);
            list_destroy_and_destroy_elements(swap_pages_info, free);
            send_confirmation_package(client_fd, -1);
            return;
        }
        
        // Escribir todas las páginas al archivo de swap
        if (swap_write_pages(swap_pages_info, process_memory, memoria_config.TAM_PAGINA)) {
            proc->is_suspended = true;
            proc->swap_pages_info = swap_pages_info;
            
            // Liberar frames físicos
            if (proc->allocated_frames != NULL) {
                frame_free_frames(proc->allocated_frames);
                proc->allocated_frames = NULL;
            }
            
            lock_process_metrics();
            proc->metrics->swap_out_count++;
            unlock_process_metrics();
            
            LOG_OBLIGATORIO("## SWAP - PID: %u - Swap out completado (%u páginas swapeadas)", pid, total_pages);
            send_confirmation_package(client_fd, 0);
        } else {
            LOG_ERROR("SWAP: Error escribiendo páginas al archivo de swap para PID %u", pid);
            swap_free_pages(pid);
            list_destroy_and_destroy_elements(swap_pages_info, free);
            send_confirmation_package(client_fd, -1);
        }
    } else {
        LOG_ERROR("SWAP: Error leyendo memoria del proceso PID %u", pid);
        swap_free_pages(pid);
        list_destroy_and_destroy_elements(swap_pages_info, free);
        send_confirmation_package(client_fd, -1);
    }
    
    free(process_memory);
}

void handle_page_walk_request(int socket, t_package *package)
{
    // 1. Deserializar la petición de la CPU
    package->buffer->offset = 0;
    page_entry_request_data request = read_page_entry_request_package(package); // (pid, table_id, entry_index)
    LOG_INFO("PAGE_WALK: Recibida solicitud - PID: %u, Table_ID: %u, Entry_Index: %u", request.pid, request.table_id, request.entry_index);

    // 2. Encontrar el proceso y su tabla de páginas raíz
    process_info *proc = NULL;
    
    // Caso especial para PID 0 (tabla de páginas de kernel o sistema)
    if (request.pid == 0) {
        LOG_DEBUG("PAGE_WALK: Solicitando acceso a tabla con PID 0 (kernel/sistema)");
        
        // Buscar entre todos los procesos activos cuál tiene la tabla solicitada
        lock_process_list();
        
        t_list* procesos = process_manager_get_process_list();
        LOG_DEBUG("PAGE_WALK: Hay %d procesos en lista global para buscar la tabla %u", 
                 list_size(procesos), request.table_id);
        
        // Primera pasada: Mostrar todos los procesos y sus tablas raíz
        LOG_DEBUG("PAGE_WALK: Listado de procesos en memoria y sus tablas raíz:");
        for (int i = 0; i < list_size(procesos); i++) {
            process_info *current = list_get(procesos, i);
            if (current && current->page_table) {
                LOG_DEBUG("PAGE_WALK: Proceso PID %d - Tabla raíz ID: %u", 
                         current->pid, current->page_table->table_id);
            } else {
                LOG_DEBUG("PAGE_WALK: Proceso índice %d es NULL o no tiene tabla", i);
            }
        }
        
        // Segunda pasada: Buscar la tabla con el ID solicitado
        for (int i = 0; i < list_size(procesos); i++) {
            process_info *current = list_get(procesos, i);
            if (current && current->page_table) {
                LOG_DEBUG("PAGE_WALK: Buscando tabla %u en el proceso %d", 
                         request.table_id, current->pid);
                t_page_table *found = find_page_table_by_id_recursive(current->page_table, request.table_id);
                if (found) {
                    proc = current;
                    LOG_INFO("PAGE_WALK: ¡Tabla %u encontrada en proceso %d!", 
                            request.table_id, current->pid);
                    break;
                } else {
                    LOG_DEBUG("PAGE_WALK: No se encontró la tabla %u en el proceso %d", 
                             request.table_id, current->pid);
                }
            }
        }
        unlock_process_list();
        
        if (proc == NULL) {
            LOG_ERROR("PAGE_WALK: No se pudo encontrar la tabla %u en ningún proceso para PID 0", 
                     request.table_id);
        }
    } else {
        // Búsqueda normal para PIDs distintos de 0
        proc = process_manager_find_process(request.pid);
        
        if (proc != NULL && proc->page_table != NULL) {
            LOG_DEBUG("PAGE_WALK: Proceso %u encontrado, tabla raíz ID: %u", 
                     proc->pid, proc->page_table->table_id);
        } else {
            LOG_DEBUG("PAGE_WALK: Proceso %u no encontrado o sin tabla de páginas", request.pid);
        }
    }
    
    if (proc == NULL || proc->page_table == NULL)
    {
        LOG_ERROR("PAGE_WALK: Proceso %u o su tabla de páginas no encontrada.", request.pid);
        // Enviar respuesta de error
        send_page_entry_response_package(socket, 0, 2); // 2 = error
        return;
    }

    // Incrementar métricas (si aplica)
    if (proc->metrics) {
        lock_process_metrics(); 
        proc->metrics->page_table_access_count++; 
        unlock_process_metrics();
    }

    lock_page_table(); // Proteger acceso a las tablas de páginas

    // 3. Encontrar la tabla específica usando su ID
    LOG_DEBUG("PAGE_WALK: Buscando tabla ID %u en proceso PID %u (tabla raíz ID: %u)", 
             request.table_id, proc->pid, proc->page_table->table_id);
             
    // Si estamos buscando la tabla raíz (ID=0), devolvemos directamente
    t_page_table *target_table = NULL;
    if (request.table_id == 0) {
        LOG_DEBUG("PAGE_WALK: Solicitada tabla raíz (ID=0), acceso directo");
        target_table = proc->page_table;
    } else {
        // Buscar recursivamente en el árbol de tablas
        target_table = find_page_table_by_id_recursive(proc->page_table, request.table_id);
    }

    if (target_table == NULL)
    {
        LOG_ERROR("PAGE_WALK: Tabla con ID %u no encontrada para PID %u.", request.table_id, request.pid);
        unlock_page_table();
        send_page_entry_response_package(socket, 0, 2); // 2 = error
        return;
    }
    
    LOG_DEBUG("PAGE_WALK: Tabla ID %u encontrada, contiene %u entradas",
             target_table->table_id, (uint32_t)list_size(target_table->entries));

    // 4. Obtener la entrada específica de esa tabla
    if (request.entry_index >= list_size(target_table->entries))
    {
        LOG_ERROR("PAGE_WALK: Índice de entrada %u fuera de rango para la tabla %u (máximo: %u).", 
                 request.entry_index, request.table_id, (uint32_t)(list_size(target_table->entries) - 1));
        unlock_page_table();
        send_page_entry_response_package(socket, 0, 2); // 2 = error
        return;
    }
    
    t_page_table_entry *entry = (t_page_table_entry *)list_get(target_table->entries, request.entry_index);
    if (entry == NULL) {
        LOG_ERROR("PAGE_WALK: La entrada %u de la tabla %u es NULL", 
                 request.entry_index, request.table_id);
        unlock_page_table();
        send_page_entry_response_package(socket, 0, 2); // 2 = error
        return;
    }

    // 5. Determinar la respuesta y enviarla
    uint32_t response_value;
    bool is_frame;

    if (entry->is_last_level)
    {
        response_value = entry->frame_number;
        is_frame = true;
        LOG_INFO("PAGE_WALK: PID %u, Tabla %u, Entrada %u es último nivel. Devolviendo Frame: %u", 
                request.pid, request.table_id, request.entry_index, response_value);
    }
    else
    {
        response_value = entry->next_table_id;
        is_frame = false;
        LOG_INFO("PAGE_WALK: PID %u, Tabla %u, Entrada %u no es último nivel. Devolviendo Next_Table_ID: %u (next_table=%p)", 
                request.pid, request.table_id, request.entry_index, response_value, (void*)entry->next_table);
    }

    unlock_page_table();

    send_page_entry_response_package(socket, response_value, is_frame ? 0 : 1);
}

/**
 * @brief Busca recursivamente una tabla de páginas por su ID dentro de la jerarquía de un proceso.
 *
 * @param current_table La tabla desde la cual comenzar la búsqueda.
 * @param target_id El ID de la tabla que se está buscando.
 * @return Un puntero a la t_page_table encontrada, o NULL si no se encuentra.
 */
t_page_table *find_page_table_by_id_recursive(t_page_table *current_table, uint32_t target_id)
{
    if (current_table == NULL)
    {
        LOG_DEBUG("PAGE_WALK: find_page_table_by_id_recursive - Tabla actual es NULL");
        return NULL;
    }    LOG_DEBUG("PAGE_WALK: Revisando tabla con ID %u (buscando %u), tiene %u entradas",
             current_table->table_id, target_id, (uint32_t)list_size(current_table->entries));

    // Caso base: Encontramos la tabla
    if (current_table->table_id == target_id)
    {
        LOG_DEBUG("PAGE_WALK: ¡Tabla con ID %u encontrada!", target_id);
        return current_table;
    }

    // Paso recursivo: Buscar en las sub-tablas
    for (int i = 0; i < list_size(current_table->entries); i++)
    {
        t_page_table_entry *entry = (t_page_table_entry *)list_get(current_table->entries, i);
        if (entry != NULL && !entry->is_last_level)
        {
            LOG_DEBUG("PAGE_WALK: Revisando entrada %d, next_table_id=%u, is_last_level=%s, next_table=%p", 
                     i, entry->next_table_id, entry->is_last_level ? "true" : "false", 
                     (void*)entry->next_table);
            
            // Verificamos primero si la entrada tiene el ID que buscamos
            if (entry->next_table_id == target_id && entry->next_table != NULL)
            {
                LOG_DEBUG("PAGE_WALK: Tabla con ID %u encontrada directamente en entry[%d]", target_id, i);
                return entry->next_table;
            }
            
            // Llamada recursiva usando el puntero interno
            if (entry->next_table != NULL) {
                t_page_table *found_table = find_page_table_by_id_recursive(entry->next_table, target_id);
                if (found_table != NULL)
                {
                    LOG_DEBUG("PAGE_WALK: Tabla con ID %u encontrada en subárbol de entry[%d]", target_id, i);
                    return found_table; // Si se encontró en esta rama, la devolvemos
                }
            } else {
                LOG_DEBUG("PAGE_WALK: Entrada %d tiene next_table=NULL pero no es último nivel", i);
            }
        } else if (entry != NULL) {
            LOG_DEBUG("PAGE_WALK: Entrada %d es de último nivel o NULL", i);
        }
    }

    LOG_DEBUG("PAGE_WALK: No se encontró la tabla con ID %u en el árbol de la tabla %u", 
             target_id, current_table->table_id);
    return NULL; // No se encontró en esta rama del árbol
}