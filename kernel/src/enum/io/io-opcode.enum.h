#ifndef ENUM_IO_OPCODE
#define ENUM_IO_OPCODE

#define IO_OPCODE_ENUM_SIZE 1

typedef enum {
    HANDSHAKE,
} t_io_opcode;

t_io_opcode io_opcode_from_string(char*);

#endif