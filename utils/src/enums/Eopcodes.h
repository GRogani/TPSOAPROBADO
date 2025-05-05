#ifndef ENUM_OPCODES_H
#define ENUM_OPCODES_H

/**
 * @enum OPCODE
 * @brief Códigos de operación para paquetes de comunicación
 * @note - `HANDSHAKE`
 */
typedef uint32_t OPCODE;
enum {
    HANDSHAKE,
    CPU_PID,
    CPU_PC,
    CPU_REQUEST_INSTRUCTION,
    CPU_RESPONSE_INSTRUCTION,
    CPU_WRITE_MEMORY_REQUEST,
    CPU_READ_MEMORY_REQUEST,
    CPU_READ_MEMORY_RESPONSE,
    CPU_INTERRUPT_REQUEST,
    CPU_INTERRUPT_RESPONSE,
};


#endif