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
    OTHER = 1,
};


#endif