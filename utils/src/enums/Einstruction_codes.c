#include "Einstruction_codes.h"

char *instruction_code_to_string(INSTRUCTION_CODE code)
{
    switch (code)
    {
    case NOOP:
        return "NOOP";
    case WRITE:
        return "WRITE";
    case READ:
        return "READ";
    case GOTO:
        return "GOTO";
    case IO:
        return "IO";
    case INIT_PROC:
        return "INIT_PROC";
    case DUMP_MEMORY:
        return "DUMP_MEMORY";
    case EXIT:
        return "EXIT";
    default:
        return "UNKNOWN";
    }
}

static char *instruction_code_names[8] = {"NOOP",
                                          "WRITE",
                                          "READ",
                                          "GOTO",
                                          "IO",
                                          "INIT_PROC",
                                          "DUMP_MEMORY",
                                          "EXIT"};
INSTRUCTION_CODE string_to_instruction_code(char *instruction_code)
{
    if (instruction_code == NULL) return -1;

    for (int i = 0; i < INSTRUCTION_CODE_ENUM_SIZE; i++)
    {
        if (strcasecmp(instruction_code, instruction_code_names[i]) == 0)
        {
            return i;
        }
    }

    return -1; // Invalid instruction code
}
