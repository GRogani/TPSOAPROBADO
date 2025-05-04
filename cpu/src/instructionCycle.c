#include "instructionCycle.h"

instruction_t* fetch(int socket, int PC) {
    request_instruction(socket, PC);
    instruction_t* instruction = receive_instruction(socket);
    if (instruction == NULL) {
       log_error(get_logger(), "Failed to fetch instruction");
        return NULL;
    }
    return instruction;
}

int decode_execute(instruction_t* instruction, int socket_memory, int* pc) {
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
            //TODO IMPRIMIR EN LOG OBLIGATORIO
            break;
        case GOTO:
            *pc = list_get(instruction->operands, 0);
            break;
        case IO:
            // Handle IO operation
            break;
        case INIT_PROC:
            // Handle INIT_PROC operation
            break;
        case DUMP_PROCESS:
            // Handle DUMP_PROCESS operation
            break;
        case EXIT:
            // Handle EXIT operation
            break;
        default:
            LOG_ERROR("Unknown instruction: %d", instruction->instruction);
            return -1;
    }
    return 0;
}

void* MMU(uint32_t direccion_logica) {
    //TODO Simulate MMU logic
    return (void*) direccion_logica;
}