#ifndef  ENUM_INSTRUCTION_CODES_H
#define ENUM_INSTRUCTION_CODES_H

#include <stdint.h>
#include <stddef.h>
#include <strings.h>

#define INSTRUCTION_CODE_ENUM_SIZE 8

typedef uint32_t INSTRUCTION_CODE;
enum{
    NOOP,
    WRITE,
    READ,
    GOTO,
    IO,
    INIT_PROC,
    DUMP_PROCESS,
    EXIT,
    UNKNOWN = -1
};

char* instruction_code_to_string(INSTRUCTION_CODE code);
INSTRUCTION_CODE string_to_instruction_code(char *code);

#endif