#include "memoria.h"

t_list* load_script_lines(char* path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "src/program/%s", path);
    
    LOG_INFO("Intentando abrir archivo: %s", full_path);
    
    FILE *file = fopen(full_path, "r");
    if (!file) {
        LOG_ERROR("Error al abrir archivo: %s - Error: %s", full_path, strerror(errno));
        return NULL;
    }
    
    LOG_INFO("Archivo abierto correctamente: %s", full_path);

    t_list* list = list_create();
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        // Remove trailing whitespace and newline
        string_trim_right(&line);
        if (strlen(line) > 0)
            list_add(list, strdup(line));
    }

    free(line);
    fclose(file);
    return list;
}

void create_process(int socket, t_package* package) {
    t_memory_create_process* create_process_args = read_memory_create_process_request(package);
    int result = create_process_in_memory(create_process_args->pid, create_process_args->size, create_process_args->pseudocode_path);
    destroy_memory_create_process(create_process_args);
    send_memory_create_process_response(socket, 0); // 0 indicates success
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

int create_process_in_memory(uint32_t pid, uint32_t size, char* script_path) {
   
    proc_memory* proc = malloc(sizeof(proc_memory));
    proc->pid = pid;
    proc->process_size = size;
    proc->instructions = load_script_lines(script_path);

    list_add(global_memory.processes, proc);

    // TODO: ir restando el size total de la memoria
    // si se queda sin size disponible para crear el proceso, entonces debemos devolver -1
    // indicando error y retornamos ese error al kernel en forma de respuesta socket

    LOG_INFO("## PID: %d - Process Created - Size: %d\n", pid, size);

    return 0;
}



void get_instruction(int socket, t_package* package) {
    t_memory_get_instruction_request* request = read_memory_get_instruction_request(package);
    uint32_t pid = request->pid;
    uint32_t pc = request->pc;
    destroy_memory_get_instruction_request(request);

    // TODO: analizar concurrencia y si hay que aplicar semaforos
    proc_memory* proc = find_process_by_pid(pid);
    if (proc && pc <= list_size(proc->instructions)) 
    {
        char* instruction = list_get(proc->instructions, pc);
        LOG_INFO("## PID: %u - Get Instruction: %u - Instruction: %s\n", pid, pc, instruction);
        send_memory_get_instruction_response(socket, instruction);
    }
    
}

void get_instructions(int socket, t_buffer* request_buffer) {
    uint32_t pid = buffer_read_uint32(request_buffer);

    proc_memory* proc = find_process_by_pid(pid);
    if (proc) 
    {
        t_buffer* response_buffer = buffer_create(0);

        uint32_t instr_count = list_size(proc->instructions);
        buffer_add_uint32(response_buffer, instr_count);  // Enviar cantidad de instrucciones

        for (uint32_t i = 0; i < instr_count; i++) {
            char* instr = list_get(proc->instructions, i);
            buffer_add_string(response_buffer, strlen(instr) + 1, instr); 
            LOG_ERROR("## PID: %u - Instrucción %u: %s", pid, i, instr);
        }

        t_package* response = package_create(GET_INSTRUCTION, response_buffer);
        send_package(socket, response);
        package_destroy(response);
    }else{
        LOG_ERROR("## PID: %u - Proceso no encontrado.", pid);
    }

    
}


void get_free_space(int socket) {
    uint32_t mock_free = 2048;

    t_buffer* buffer = buffer_create(0);
    buffer_add_uint32(buffer, mock_free);

    t_package* package = package_create(GET_FREE_SPACE, buffer);
    send_package(socket, package);
    package_destroy(package);
}