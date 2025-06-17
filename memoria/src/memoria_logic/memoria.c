#include "memoria.h" 

global_memory_state global_memory;

void init_global_memory_state(t_memoria_config* config) {
    global_memory.config = config;
    global_memory.processes = list_create();
    pthread_mutex_init(&global_memory.processes_mutex, NULL);
    global_memory.main_logger = log_create("memoria.log", "MEMORIA_MODULE", true, config->LOG_LEVEL);

    LOG_INFO(global_memory.main_logger, "Inicializando módulo Memoria.");

    if (!memory_manager_init(config)) {
        LOG_ERROR(global_memory.main_logger, "Error crítico al inicializar espacio de memoria de usuario. Terminando.");
    }
    // TODO: Initialize SWAP manager here
    LOG_INFO(global_memory.main_logger, "Módulo Memoria inicializado.");
}

void destroy_global_memory_state() {
    LOG_INFO(global_memory.main_logger, "Destruyendo módulo Memoria.");
    list_destroy_and_destroy_elements(global_memory.processes, destroy_proc_memory);
    pthread_mutex_destroy(&global_memory.processes_mutex);
    memory_manager_destroy(); // Free user memory space
    // TODO: Destroy SWAP resources
    log_destroy(global_memory.main_logger);
}

// --- Process Management Functions ---
proc_memory* find_process_by_pid(uint32_t pid) {
    // This function assumes 'global_memory.processes_mutex' is already locked by the caller
    for (int i = 0; i < list_size(global_memory.processes); i++) {
        proc_memory* proc = list_get(global_memory.processes, i);
        if (proc->pid == pid) {
            return proc;
        }
    }
    return NULL;
}

t_list* load_script_lines(char* path) {
    char full_path[512];
    // This assumes `src/program/` relative to where your executable runs. Adjust as needed.
    snprintf(full_path, sizeof(full_path), "src/program/%s", path);

    LOG_INFO(global_memory.main_logger, "Intentando abrir archivo: %s", full_path);

    FILE *file = fopen(full_path, "r");
    if (!file) {
        LOG_ERROR(global_memory.main_logger, "Error al abrir archivo: %s", full_path);
        return NULL;
    }

    LOG_INFO(global_memory.main_logger, "Archivo abierto correctamente: %s", full_path);

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

    free(line); // Free the buffer allocated by getline
    fclose(file);
    return list;
}

int create_process(uint32_t pid, uint32_t size, char* script_path) {
    proc_memory* proc = malloc(sizeof(proc_memory));
    if (proc == NULL) {
        LOG_ERROR(global_memory.main_logger, "## PID: %u - Error: No se pudo asignar memoria para proc_memory.", pid);
        return -1;
    }

    proc->pid = pid;
    proc->process_size = size;
    proc->instructions = load_script_lines(script_path);

    // If script_path is not empty and loading failed
    if (script_path && strcmp(script_path, "") != 0 && proc->instructions == NULL) {
        LOG_ERROR(global_memory.main_logger, "## PID: %u - Error al cargar pseudocódigo desde: %s", pid, script_path);
        free(proc);
        return -1;
    }

    // Initialize page table for the process
    proc->page_table = init_page_table(global_memory.config);
    if (proc->page_table == NULL) {
        LOG_ERROR(global_memory.main_logger, "## PID: %u - Error al inicializar tabla de páginas.", pid);
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        free(proc);
        return -1;
    }

    // Initialize metrics for the process
    proc->metrics = malloc(sizeof(t_process_metrics));
    if (proc->metrics == NULL) {
        LOG_ERROR(global_memory.main_logger, "## PID: %u - Error: No se pudo asignar memoria para métricas.", pid);
        if (proc->instructions) list_destroy_and_destroy_elements(proc->instructions, free);
        destroy_page_table(proc->page_table);
        free(proc);
        return -1;
    }
    // Initialize all metrics to 0
    memset(proc->metrics, 0, sizeof(t_process_metrics)); // Set all to 0 efficiently

    // TODO: Implement actual frame allocation in user_memory_space for the process.
    // This would involve a frame management system (e.g., a bitmap or free list)
    // and updating the last-level page table entries (proc->page_table->entries->...) with frame numbers.
    // Check if (size / PAGE_SIZE) pages can be allocated. If not, return -1.

    lock_global_processes();
    list_add(global_memory.processes, proc);
    unlock_global_processes();

    LOG_INFO(global_memory.main_logger, "## PID: %u - Proceso Creado. Tamaño: %u. Tabla de Páginas inicializada.", pid, size);

    return 0; // Return 0 for success
}

void destroy_proc_memory(void* proc_void_ptr) {
    proc_memory* proc = (proc_memory*) proc_void_ptr;
    if (proc == NULL) {
        return;
    }

    LOG_INFO(global_memory.main_logger, "## PID: %u - Liberando recursos del proceso.", proc->pid);

    // 1. Log metrics (as per requirement)
    LOG_INFO(global_memory.main_logger, "## PID: %u - Métricas Finales:", proc->pid);
    LOG_INFO(global_memory.main_logger, "  - Accesos a Tablas de Páginas: %u", proc->metrics->page_table_access_count);
    LOG_INFO(global_memory.main_logger, "  - Instrucciones Solicitadas: %u", proc->metrics->instruction_requests_count);
    LOG_INFO(global_memory.main_logger, "  - Bajadas a SWAP: %u", proc->metrics->swap_out_count);
    LOG_INFO(global_memory.main_logger, "  - Subidas a Memoria Principal: %u", proc->metrics->swap_in_count);
    LOG_INFO(global_memory.main_logger, "  - Lecturas de Memoria: %u", proc->metrics->memory_read_count);
    LOG_INFO(global_memory.main_logger, "  - Escrituras de Memoria: %u", proc->metrics->memory_write_count);

    // 2. Free instructions (each line was strdup'd)
    if (proc->instructions) {
        list_destroy_and_destroy_elements(proc->instructions, free);
    }

    // 3. Destroy the page table recursively
    if (proc->page_table) {
        destroy_page_table(proc->page_table);
    }

    // 4. Free metrics
    if (proc->metrics) {
        free(proc->metrics);
    }

    // TODO: Mark frames allocated to this process as free in your frame allocator
    // This is crucial for proper memory management.

    // TODO: Mark corresponding entries in SWAP as free (if implemented)

    // 5. Free the proc_memory structure itself
    free(proc);
}


// --- Server Request Handlers ---

void init_process(int socket, t_package* package) {
    init_process_package_data* init_process_args = read_init_process_package(package);

    LOG_INFO(global_memory.main_logger, "## PID: %u - Solicitud de creación de Proceso Recibida.", init_process_args->pid);

    int result = create_process(init_process_args->pid, init_process_args->size, init_process_args->pseudocode_path);

    destroy_init_process_package(init_process_args);

    send_confirmation_package(socket, result); // 0 indicates success, -1 indicates error
}

void get_instruction(int socket, t_package* package) {
    fetch_package_data request = read_fetch_package(package);
    uint32_t pid = request.pid;
    uint32_t pc = request.pc;

    lock_global_processes();
    proc_memory* proc = find_process_by_pid(pid);

    if (proc != NULL) {
        proc->metrics->instruction_requests_count++; // Increment instruction request metric

        if (proc->instructions && pc < list_size(proc->instructions)) {
            char* instruction = list_get(proc->instructions, pc);
            if (instruction) {
                LOG_INFO(global_memory.main_logger, "## PID: %u - Obtener Instrucción: %u - Instrucción: %s", pid, pc, instruction);
                send_instruction_package(socket, instruction);
            } else {
                LOG_ERROR(global_memory.main_logger, "## PID: %u - PC %u la instrucción es NULL.", pid, pc);
                send_instruction_package(socket, "");
            }
        } else {
            LOG_ERROR(global_memory.main_logger, "## PID: %u - PC %u fuera de límites (máx: %d).", pid, pc,
                      proc->instructions ? list_size(proc->instructions) : 0);
            send_instruction_package(socket, "");
        }
    } else {
        LOG_ERROR(global_memory.main_logger, "## PID: %u - Proceso no encontrado.", pid);
        send_instruction_package(socket, "");
    }

    unlock_global_processes();
}

void delete_process(int socket, t_package *package) {
    uint32_t pid_to_delete = read_kill_process_package(package);

    LOG_INFO(global_memory.main_logger, "## PID: %u - Solicitud de Finalización de Proceso Recibida.", pid_to_delete);

    lock_global_processes();
    int found_index = -1;
    for (int i = 0; i < list_size(global_memory.processes); i++) {
        proc_memory* proc = list_get(global_memory.processes, i);
        if (proc->pid == pid_to_delete) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // list_remove_and_destroy_element will call destroy_proc_memory on the removed element
        list_remove_and_destroy_element(global_memory.processes, found_index, destroy_proc_memory);
        LOG_INFO(global_memory.main_logger, "## PID: %u - Proceso Finalizado y recursos liberados.", pid_to_delete);
        send_confirmation_package(socket, 0); // Success
    } else {
        LOG_ERROR(global_memory.main_logger, "## PID: %u - Intento de finalizar proceso no existente.", pid_to_delete);
        send_confirmation_package(socket, -1); // Error
    }
    unlock_global_processes();
}

void get_free_space(int socket) {
    // TODO: Replace with actual calculation of free space
    uint32_t mock_free = 2048; // Placeholder

    t_buffer* buffer = buffer_create(sizeof(uint32_t)); // Allocate for actual data
    buffer_add_uint32(buffer, mock_free);

    t_package* package = create_package(0, buffer); // Replace 0 with your actual package type for free space
    send_package(socket, package);
    destroy_package(package);
    LOG_INFO(global_memory.main_logger, "DUMMY: Enviando espacio libre mock: %u bytes.", mock_free);
}