#ifndef DTOS_INSTRUCTIONS_H
#define DTOS_INSTRUCTIONS_H

#include "../utils.h"

typedef struct
{
    INSTRUCTION_CODE instruction_code;
    uint32_t operand_numeric1;
    uint32_t operand_numeric2;
    uint32_t operand_string_size;
    char* operand_string;
} t_instruction;

t_instruction* create_instruction(INSTRUCTION_CODE instruction_code, uint32_t operand_numeric1, uint32_t operand_numeric2, uint32_t operand_string_size, const char* operand_string);

void send_PID_PC(int socket_dispatch, uint32_t PID, uint32_t PC);

void send_instruction_request(int socket_dispatch, uint32_t PID, uint32_t PC);

void send_instruction(int socket_dispatch, t_instruction* instruction);


/// @brief Cambia el opcode del paquete a SYSCALL y lo envía por el socket.
/// @param socket Donde enviar el paquete.
/// @param package Paquete que se enviará con el opcode cambiado a SYSCALL.
void send_syscall(int socket, t_package* package);

#endif