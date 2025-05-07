#include "instructionCycle.h"

t_package* fetch(int socket, int PC) {
    request_instruction(socket, PC);
    t_package* instruction_package = receive_instruction(socket);
    if (instruction == NULL) {
       log_error(get_logger(), "Failed to fetch instruction");
        return NULL;
    }
    return instruction_package;
}

instruction_t* decode(t_package* package) {
    instruction_t* instruction = malloc(sizeof(instruction_t));
    instruction->cod_instruction = buffer_read_uint32(package->buffer);
    instruction->operands = list_create();
    
    switch(instruction->cod_instruction){
        case NOOP:
            break;
        case WRITE:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            instruction->operand_string_size = buffer_read_uint32(package->buffer);
            instruction->operand_string = buffer_read_string(package->buffer, instruction->operand_string_size);
        case READ:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            instruction->operand_numeric2 = buffer_read_uint32(package->buffer);
            break;
        case GOTO:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            break;
        default:
            break;
    }

    return instruction;
}

int execute(instruction_t* instruction, t_package* instruction_package, int socket_memory, int socket_dispatch, int* pc) {
    switch (instruction->cod_instruction) {
        case NOOP:
            // No operation
            break;
        case WRITE:
             uint32_t logic_dir_write = instruction->operand_numeric1;
             char* valor_write = instruction->operand_string;
             void* physic_dir_write = MMU(logic_dir_write);
            write_memory_request(socket_memory, (uint32_t) physic_dir_write, valor_write);
            break;
        case READ:
            uint32_t logic_dir_read = instruction->operand_numeric1;
            uint32_t size = instruction->operand_numeric2;
            void* physic_dir_read = MMU(logic_dir_read);
            read_memory_request(socket_memory, physic_dir_read, size);
            char* data = read_memory_response(socket_memory);
            log_info(get_logger(), "Data read from memory: %s", data);
            break;
        case GOTO:
            *pc = instruction->operand_numeric1;
            return 0;
            break;
        case IO:
            send_package(socket_dispatch, instruction_package);
            break;
        case INIT_PROC:
            send_package(socket_dispatch, instruction_package);
            break;
        case DUMP_PROCESS:
            send_package(socket_dispatch, instruction_package);
            break;
        case EXIT:
            send_package(socket_dispatch, instruction_package);
            break;
        default:
            log_error(get_logger(), "Unknown instruction: %d", instruction->cod_instruction);
            return -1;
    }
    pc++;
    return 0;
}

int check_interrupt(int socket_interrupt, int pid_on_execute, int pc_on_execute) {
    t_package* package = recv_package(socket_interrupt);
    if(package->opcode == CPU_INTERRUPT_REQUEST) {
        log_info(get_logger(), "Received interrupt from kernel");
        int pid_received = buffer_read_uint32(package->buffer);
        if(pid_received == pid_on_execute) {
            log_info(get_logger(), "Interrupt for PID %d received", pid_received);
            t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
            buffer_add_uint32(buffer, pid_received);
            buffer_add_uint32(buffer, pc_on_execute);
            t_package* package = package_create(CPU_INTERRUPT_RESPONSE, buffer);
            send_package(socket_interrupt, package);
            package_destroy(package);
            log_info(get_logger(), "Interrupt for PID %d executed", pid_received);
            return 1;
        } else {
            log_info(get_logger(), "Interrupt for PID %d received, but not executing", pid_received);
        }

        
        
    } else {
        log_error(get_logger(), "Received unexpected package: %d", package->opcode);
    }
    return 0;

}

int MMU(uint32_t logic_dir) {
    t_config* config_memoria = init_config("../memoria/memoria.config");
    int levels = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    int entrys_by_table = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    int size_pag = config_get_int_value(config_memoria, "TAM_PAGINA");

    int num_page = logic_dir / size_pag;
    int offset = logic_dir % size_pag;
    int physic_dir = 0;
    
    for(int actual_level = 1; actual_level <= levels; actual_level++) {
        int divisor = pow(entrys_by_table, levels - actual_level);
        int entry = (num_page / divisor) % entrys_by_table;
        physic_dir = physic_dir * entrys_by_table + entry;
    }
    
    return physic_dir * size_pag + offset;
}