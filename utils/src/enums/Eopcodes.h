#ifndef ENUM_OPCODES_H
#define ENUM_OPCODES_H

/**
 * @enum OPCODE
 * @brief Códigos de operación para paquetes de comunicación
 * @note - `HANDSHAKE`
 */
typedef uint32_t OPCODE;
enum {
    // all
    HANDSHAKE,

    // kernel -> cpu
    PID_PC_PACKAGE,
    CPU_INTERRUPT_REQUEST,
    
    // cpu -> memoria
    CPU_REQUEST_INSTRUCTION,
    CPU_WRITE_MEMORY_REQUEST,
    CPU_READ_MEMORY_RESPONSE,

    // memoria -> cpu
    CPU_RESPONSE_INSTRUCTION,
    CPU_READ_MEMORY_REQUEST,

    // cpu -> kernel
    CPU_INTERRUPT_RESPONSE,

    // kernel -> io
    REQUEST_IO,

    // io -> kernel
    IO_COMPLETION,

    // memoria <> kernel/cpu
    OBTENER_ESPACIO_LIBRE,
    CREAR_PROCESO,
    OBTENER_INSTRUCCION,
    LISTA_INSTRUCCIONES,
};

#endif