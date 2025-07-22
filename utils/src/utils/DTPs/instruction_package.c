#include "instruction_package.h"

t_package* create_instruction_package(char* instruction) 
{
    t_buffer *buffer = buffer_create(0);
    buffer_add_string(buffer, strlen(instruction) + 1, instruction);
    t_package *response = create_package(INSTRUCTION, buffer);
    return response;
}

int send_instruction_package(int socket, char* instruction) 
{
    LOG_PACKAGE("Sending instruction package: instruction: %s", instruction);
    t_package* package = create_instruction_package(instruction);
    int bytes_sent = send_package(socket, package);
    destroy_package(package);
    return bytes_sent;
}

char* read_instruction_package(t_package* package) 
{
    package->buffer->offset = 0;
    int32_t instruction_len;
    char* instruction = buffer_read_string(package->buffer, &instruction_len);
    LOG_PACKAGE("Read instruction package: instruction: %s", instruction);
    return instruction;
}