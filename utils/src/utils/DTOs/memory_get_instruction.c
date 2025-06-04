#include "memory_get_instruction.h"

// REQUEST

// used by memory
t_get_instruction read_memory_get_instruction_package(t_package* package) 
{
    t_get_instruction request;
    package->buffer->offset = 0;   
    request.pid = buffer_read_uint32(package->buffer);
    request.pc = buffer_read_uint32(package->buffer);
    
    return request;
}

// used by cpu
t_package* create_memory_get_instruction_package(uint32_t pid, uint32_t pc) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, pid);
    buffer_add_uint32(buffer, pc);
    return package_create(GET_INSTRUCTION, buffer);
}

void send_memory_get_instruction_package(int socket, uint32_t pid, uint32_t pc) 
{
    t_package* package = create_memory_get_instruction_package(pid, pc);
    send_package(socket, package);
    package_destroy(package);
}

// RESPONSE

// used by cpu
char* read_memory_instruction_package(t_package* package) 
{
    package->buffer->offset = 0;
    uint32_t instruction_len;
    char* instruction = buffer_read_string(package->buffer, &instruction_len);
    return instruction;
}

t_package* create_memory_instruction_package(char* instruction) 
{
    t_buffer *buffer = buffer_create(0);
    buffer_add_string(buffer, strlen(instruction) + 1, instruction);
    t_package *response = package_create(GET_INSTRUCTION, buffer);
    return response;
}

// used by memory
int send_memory_instruction_package(int socket, char* instruction) 
{
    t_package* package = create_memory_instruction_package(instruction);
    int bytes_sent = send_package(socket, package);
    package_destroy(package);
    return bytes_sent;
}