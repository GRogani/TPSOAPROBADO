#ifndef ENUM_OPCODES_H
#define ENUM_OPCODES_H

#include <stdint.h>

typedef uint32_t OPCODE;
enum
{
    FETCH,              // cpu -> memoria
    INSTRUCTION,        // memoria -> cpu
    LIST_INSTRUCTIONS,
    GET_FREE_SPACE,
    INIT_PROCESS,       // kernel -> memoria
    UNSUSPEND_PROCESS,
    KILL_PROCESS,
    SWAP,
    WRITE_MEMORY,
    READ_MEMORY,

    CONFIRMATION,       // server -> client // TODO: ???  PODEMOS USAR EL MISMO KILL_PROCESS pero de retorno, es decir, que lo lea el kernel en vez de la memoria. no es necesario crear OPCODES nuevos...

    NEW_IO,             // io -> kernel
    IO_COMPLETION,      // io -> kernel
    SYSCALL,            // cpu -> kernel

    DISPATCH,           // cpu -> kernel
    INTERRUPT,          // kernel -> cpu
    CPU_CONTEXT,        // cpu -> kernel

    IO_OPERATION,       // cpu ->  // TODO: ???? lo que sea para tests, lo ponemos por separado porfa, sino se complica de entender. en este enum solo lo que se usa en la logica, nada de tests
};

/**
 * @brief Convierte un OPCODE a su representación en cadena.
 * @param opcode El código de operación a convertir.
 * @return Una cadena que representa el OPCODE.
 */
char* opcode_to_string(OPCODE opcode);

#endif