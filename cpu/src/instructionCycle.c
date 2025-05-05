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
    //TODO no termino de entender como deserializar la instruccion
    instruction_t* instruction = malloc(sizeof(instruction_t));
    instruction->cod_instruction = buffer_read_uint32(package->buffer);
    instruction->operands = list_create();

    while (package->buffer->offset < package->buffer->size) {
        uint32_t operand = buffer_read_uint32(package->buffer);
        list_add(instruction->operands, (void*) operand);
    }

    return instruction;
}

int execute(instruction_t* instruction, t_package* instruction_package, int socket_memory, int socket_dispatch, int* pc) {
    switch (instruction->cod_instruction) {
        case NOOP:
            // No operation
            break;
        case WRITE:
             uint32_t direccion_logica_write = (uint32_t) list_get(instruction->operands, 0);
             char* valor_write = (char*)list_get(instruction->operands, 1);
             void* direccion_fisica_write = MMU(direccion_logica_write);
            write_memory_request(socket_memory, (uint32_t) direccion_fisica_write, valor_write);
            break;
        case READ:
            uint32_t direccion_logica_read = (uint32_t) list_get(instruction->operands, 0);
            uint32_t size = (uint32_t) list_get(instruction->operands, 1);
            void* direccion_fisica_read = MMU(direccion_logica_read);
            read_memory_request(socket_memory, direccion_fisica_read, size);
            char* data = read_memory_response(socket_memory);
            log_info(get_logger(), "Data read from memory: %s", data);
            break;
        case GOTO:
            *pc = list_get(instruction->operands, 0);
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

void* MMU(uint32_t direccion_logica) {
    //TODO Simulate MMU logic
    return (void*) direccion_logica;
}