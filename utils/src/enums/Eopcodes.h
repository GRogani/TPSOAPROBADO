#ifndef ENUM_OPCODES_H
#define ENUM_OPCODES_H

/**
 * @enum OPCODE
 * @brief Códigos de operación para paquetes de comunicación
 * @note - `HANDSHAKE`
 */
typedef uint32_t OPCODE;
enum {
    // memoria opcodes
    GET_INSTRUCTION,
    LIST_INSTRUCTIONS,
    GET_FREE_SPACE,
    CREATE_PROCESS,
    WRITE_MEMORY,
    READ_MEMORY,

    // opcodes kernel
    IO_NEW_DEVICE,
    IO_COMPLETION,
    CPU_SYSCALL,

    // opcodes CPU
    PID_PC_PACKAGE,
    CPU_INTERRUPT,

    // opcodes IO
    REQUEST_IO,
};

#endif