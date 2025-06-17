#include "memoria.h"

t_list* load_script_lines(char* path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "src/program/%s", path);
    
    LOG_INFO("Intentando abrir archivo: %s", full_path);
    
    FILE *file = fopen(full_path, "r");
    if (!file) {
        LOG_ERROR("Error al abrir archivo: %s", full_path);
        return NULL;
    }
    
    LOG_INFO("Archivo abierto correctamente: %s", full_path);

    t_list* list = list_create();
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
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

void init_process(int socket, t_package* package) 
{
    init_process_package_data* init_process_args = read_init_process_package(package);

    LOG_INFO("## PID: %d - Create Process Request Received", init_process_args->pid);

    create_process(init_process_args->pid, init_process_args->size, init_process_args->pseudocode_path);

    destroy_init_process_package(init_process_args);

    send_confirmation_package(socket, 0); // 0 indicates success
}


proc_memory* find_process_by_pid(int pid) {
    proc_memory* proc = NULL;
    
    for (int i = 0; i < list_size(global_memory.processes); i++) {
       proc = list_get(global_memory.processes, i);
        if (proc->pid == pid) {
            break;
        }
    }

    return proc;
}

int create_process(uint32_t pid, uint32_t size, char* script_path) {
   
    proc_memory* proc = malloc(sizeof(proc_memory));
    proc->pid = pid;
    proc->process_size = size;
    proc->instructions = load_script_lines(script_path);

    lock_global_processes();
    list_add(global_memory.processes, proc);
    unlock_global_processes();

    // TODO: ir restando el size total de la memoria
    // si se queda sin size disponible para crear el proceso, entonces debemos devolver -1
    // indicando error y retornamos ese error al kernel en forma de respuesta socket

    LOG_INFO("## PID: %d - Process Created - Size: %d\n", pid, size);

    return 0;
}

void get_instruction(int socket, t_package* package) {
    fetch_package_data request = read_fetch_package(package);
    uint32_t pid = request.pid;
    uint32_t pc = request.pc;

    lock_global_processes();

    proc_memory *proc = find_process_by_pid(pid);
    if (proc != NULL) {
        // Verificar bounds de manera thread-safe
        if (proc->instructions && pc < list_size(proc->instructions)) {
            char* instruction = list_get(proc->instructions, pc);
            if (instruction) {
                LOG_INFO("## PID: %u - Get Instruction: %u - Instruction: %s\n", pid, pc, instruction);
                send_instruction_package(socket, instruction);
            } else {
                LOG_ERROR("## PID: %u - PC %u instruction is NULL", pid, pc);
                send_instruction_package(socket, ""); // Send empty instruction
            }
        } else {
            LOG_ERROR("## PID: %u - PC %u out of bounds (max: %d)", pid, pc, 
                     proc->instructions ? list_size(proc->instructions) : 0);
            send_instruction_package(socket, ""); // Send empty instruction
        }
    } else {
        LOG_ERROR("## PID: %u - Process not found", pid);
        send_instruction_package(socket, ""); // Send empty instruction
    }
    
    unlock_global_processes();
}

// TODO: pasarlo a un DTP
void get_free_space(int socket) {
    uint32_t mock_free = 2048;

    t_buffer* buffer = buffer_create(0);
    buffer_add_uint32(buffer, mock_free);

    t_package* package = create_package(GET_FREE_SPACE, buffer);
    send_package(socket, package);
    destroy_package(package);
}

void delete_process(int socket, t_package *package)
{





}