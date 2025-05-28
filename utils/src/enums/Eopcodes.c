#include "Eopcodes.h"

const char* opcode_to_string(OPCODE opcode) 
{
    switch (opcode) {
        // memoria
        case GET_INSTRUCTION: return "GET_INSTRUCTION";
        case LIST_INSTRUCTIONS: return "LIST_INSTRUCTIONS";
        case GET_FREE_SPACE: return "GET_FREE_SPACE";
        case CREATE_PROCESS: return "CREATE_PROCESS";
        case WRITE_MEMORY: return "WRITE_MEMORY";
        case READ_MEMORY: return "READ_MEMORY";

        // kernel
        case IO_NEW_DEVICE: return "IO_NEW_DEVICE";
        case IO_COMPLETION: return "IO_COMPLETION";
        case CPU_SYSCALL: return "CPU_SYSCALL";

        // CPU
        case PID_PC_PACKAGE: return "PID_PC_PACKAGE";
        case CPU_INTERRUPT: return "CPU_INTERRUPT";

        // IO
        case REQUEST_IO: return "REQUEST_IO";

        default: return "UNKNOWN_OPCODE";
    }
}
