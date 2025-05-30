#include "instructions.h"

t_instruction* create_instruction(INSTRUCTION_CODE instruction_code, uint32_t operand_numeric1, uint32_t operand_numeric2, uint32_t operand_string_size,  const char* operand_string) 
{
    t_instruction* instruction = malloc(sizeof(t_instruction));
    instruction->instruction_code = instruction_code;
    instruction->operand_numeric1 = operand_numeric1;
    instruction->operand_numeric2 = operand_numeric2;
    instruction->operand_string_size = operand_string_size;

    if(operand_string_size<=0)
        instruction->operand_string = NULL;
    else
        instruction->operand_string = safe_malloc(operand_string_size + 1);
    
    return instruction;
}

void send_PID_PC(int socket_dispatch, uint32_t PID, uint32_t PC) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, PID);
    buffer_add_uint32(buffer, PC);
    t_package* package = package_create(PID_PC_PACKAGE, buffer);
    send_package(socket_dispatch, package);
    package_destroy(package);
}

void send_instruction_request(int socket_dispatch, uint32_t PID, uint32_t PC) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, PID);
    buffer_add_uint32(buffer, PC);
    t_package* package = package_create(GET_INSTRUCTION, buffer);
    send_package(socket_dispatch, package);
    package_destroy(package);
}

void send_instruction(int socket_dispatch, t_instruction* instruction)
{
    t_buffer* buffer = buffer_create(sizeof(INSTRUCTION_CODE) + sizeof(uint32_t) * 3 + instruction->operand_string_size);
    buffer_add_uint32(buffer, instruction->instruction_code);
    buffer_add_uint32(buffer, instruction->operand_numeric1);
    buffer_add_uint32(buffer, instruction->operand_numeric2);
    buffer_add_uint32(buffer, instruction->operand_string_size);
    
    if (instruction->operand_string_size > 0) {
        buffer_add_string(buffer, instruction->operand_string_size, instruction->operand_string);
    }
    else
    {
        buffer_add_uint32(buffer, 0);
    }
    
    t_package* package = package_create(GET_INSTRUCTION, buffer);
    send_package(socket_dispatch, package);
    package_destroy(package);
}

void send_syscall(int socket, t_package* package)
{
    package->opcode = CPU_SYSCALL;
    send_package(socket, package);
}

