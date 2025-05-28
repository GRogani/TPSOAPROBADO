#include "memoria.h"

glb_memory global_memory;

t_list* load_script_lines(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return NULL;

    t_list* list = list_create();
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        string_trim(&line);
        if (strlen(line) > 0)
            list_add(list, strdup(line));
    }

    free(line);
    fclose(file);
    return list;
}

void create_process(int socket, t_buffer* buffer) {
    uint32_t pid = buffer_read_uint32(buffer);
    uint32_t size = buffer_read_uint32(buffer);
    uint32_t path_len;
    char* path = buffer_read_string(buffer, &path_len);

    int result = create_process_in_memory(pid, size, path);
    free(path);

    // TODO: send response to kernel using opcode CREATE_PROCESS
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

    log_info(get_logger(),"## PID: %d - Process Created - Size: %d\n", pid, size);

    return 0;
}



void get_instruction(int socket, t_buffer* request_buffer) {
    uint32_t pid = buffer_read_uint32(request_buffer);
    uint32_t pc = buffer_read_uint32(request_buffer);

    proc_memory* proc = find_process_by_pid(pid);
    if (proc && pc <= list_size(proc->instructions)) 
    {
        char* instr = list_get(proc->instructions, pc);
        log_info(get_logger(),"## PID: %u - Get Instruction: %u - Instruction: %s\n", pid, pc, instr);

        t_buffer* response_buffer = buffer_create(0);
        buffer_add_string(response_buffer, strlen(instr) + 1, instr);
        t_package* response = package_create(GET_INSTRUCTION, response_buffer);

        send_package(socket, response);
        package_destroy(response);
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
            log_info(get_logger(), "## PID: %u - Instrucción %u: %s", pid, i, instr);
        }

        t_package* response = package_create(GET_INSTRUCTION, response_buffer);
        send_package(socket, response);
        package_destroy(response);
    }else{
        log_error(get_logger(), "## PID: %u - Proceso no encontrado.", pid);
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