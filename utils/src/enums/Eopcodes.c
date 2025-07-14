#include "Eopcodes.h"

char* opcode_to_string(OPCODE opcode) 
{
    switch (opcode) {
 
        case FETCH: return "FETCH";
        case LIST_INSTRUCTIONS: return "LIST_INSTRUCTIONS";
        case GET_FREE_SPACE: return "GET_FREE_SPACE";
        case INIT_PROCESS: return "INIT_PROCESS";
        case WRITE_MEMORY: return "WRITE_MEMORY";
        case READ_MEMORY: return "READ_MEMORY";
        case C_DUMP_MEMORY: return "C_DUMP_MEMORY";


        case NEW_IO: return "NEW_IO";
        case IO_COMPLETION: return "IO_COMPLETION";
        case SYSCALL: return "SYSCALL";


        case DISPATCH: return "DISPATCH";
        case INTERRUPT: return "INTERRUPT";

        case IO_OPERATION: return "REQUEST_IO";

        default: return "UNKNOWN_OPCODE";
    }
}
