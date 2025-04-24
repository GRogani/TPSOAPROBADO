#include "instructionCycle.h"

instruction_t* fetch(int socket, int PC) {
    request_instruction(socket, PC);
    instruction_t* instruction = receive_instruction(socket);
    if (instruction == NULL) {
        LOG_ERROR("Failed to fetch instruction");
        return NULL;
    }
    return instruction;
}

int decode_execute(instruction_t* instruction) {
    switch (instruction->instruction) {
        case NOOP:
            // No operation
            break;
        case WRITE:
            // Handle WRITE operation
            break;
        case READ:
            // Handle READ operation
            break;
        case GOTO:
            // Handle GOTO operation
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