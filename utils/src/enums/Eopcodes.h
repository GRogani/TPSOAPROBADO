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
    IO = 1, //TODO: IO_REQUEST no sería del lado del modulo IO? El modulo kernel sólo le llegan los interrupts del IO (IO_OPERATION_COMPLETED)
};


#endif