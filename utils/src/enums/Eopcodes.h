#ifndef ENUM_OPCODES_H
#define ENUM_OPCODES_H

#include <stdint.h>

typedef int32_t OPCODE;
enum
{
    FETCH,       // cpu -> memoria
    INSTRUCTION, // memoria -> cpu
    LIST_INSTRUCTIONS,
    GET_FREE_SPACE,
    INIT_PROCESS,      // kernel -> memoria
    UNSUSPEND_PROCESS, // kernel -> memoria
    KILL_PROCESS,      // kernel -> memoria
    SWAP,              // kernel -> memoria
    WRITE_MEMORY,      // kernel -> memoria
    READ_MEMORY,       // kernel -> memoria
    C_DUMP_MEMORY,       // kernel -> memoria
    GET_PAGE_ENTRY,

    CONFIRMATION, // server -> client

    NEW_IO,        // io -> kernel
    IO_COMPLETION, // io -> kernel
    SYSCALL,       // cpu -> kernel

    DISPATCH,    // cpu -> kernel
    INTERRUPT,   // kernel -> cpu
    CPU_CONTEXT, // cpu -> kernel

    IO_OPERATION, // cpu -> kernel
};

/**
 * @brief Convierte un OPCODE a su representación en cadena.
 * @param opcode El código de operación a convertir.
 * @return Una cadena que representa el OPCODE.
 */
char* opcode_to_string(OPCODE opcode);

#endif