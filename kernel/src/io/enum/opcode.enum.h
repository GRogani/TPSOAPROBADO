#ifndef ENUM_IO_OPCODE_H
#define ENUM_IO_OPCODE_H

#define IO_OPCODE_ENUM_SIZE 1

typedef enum {
    HANDSHAKE,
    IO,
} t_io_opcode;

t_io_opcode io_opcode_from_string(char*);

#endif