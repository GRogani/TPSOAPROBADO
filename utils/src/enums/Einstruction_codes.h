#ifndef  ENUM_INSTRUCTION_CODES_H
#define ENUM_INSTRUCTION_CODES_H

#include <stdint.h>

typedef uint32_t INSTRUCTION_CODE;
enum{
    NOOP,
    WRITE,
    READ,
    GOTO,
    IO,
    INIT_PROC,
    DUMP_PROCESS,
    EXIT
};

char* instruction_code_to_string(INSTRUCTION_CODE code);

#endif