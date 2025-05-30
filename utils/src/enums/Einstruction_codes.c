#include  "Einstruction_codes.h"

char* instruction_code_to_string(INSTRUCTION_CODE code) {
    switch (code) {
        case NOOP: return "NOOP";
        case WRITE: return "WRITE";
        case READ: return "READ";
        case GOTO: return "GOTO";
        case IO: return "IO";
        case INIT_PROC: return "INIT_PROC";
        case DUMP_PROCESS: return "DUMP_PROCESS";
        case EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}
