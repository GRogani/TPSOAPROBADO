#ifndef ENUM_OPCODES_H
#define ENUM_OPCODES_H

/**
 * @enum OPCODE
 * @brief Códigos de operación para paquetes de comunicación
 * @note - `HANDSHAKE`
 */
typedef uint32_t OPCODE;
enum {
    HANDSHAKE = 0,
    CPU_PID_PC = 1,
    CPU_REQUEST_INSTRUCTION = 2,
    CPU_RESPONSE_INSTRUCTION = 3,
};


#endif