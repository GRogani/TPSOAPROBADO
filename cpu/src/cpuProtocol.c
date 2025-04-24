#include "cpuProtocol.h"

int* receive_PID_PC(int socket) {
    t_package* package = recv_package(socket);
    if (package == NULL) {
        LOG_ERROR("Failed to receive package");
        return NULL;
    }

    if (package->opcode != CPU_PID_PC) {
        LOG_ERROR("Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    uint32_t pid, pc;
    buffer_read(package->buffer, &pid, sizeof(uint32_t));
    buffer_read(package->buffer, &pc, sizeof(uint32_t));

    int* result = malloc(2 * sizeof(int));
    result[0] = pid;
    result[1] = pc;

    package_destroy(package);
    return result;
}

void request_instruction(int socket, int PC) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, PC);
    t_package* package = package_create(CPU_REQUEST_INSTRUCTION, buffer);
    send_package(socket, package);
    package_destroy(package);
}

instruction_t* receive_instruction(int socket) {
    t_package* package = recv_package(socket);
    if (package == NULL) {
        LOG_ERROR("Failed to receive package");
        return NULL;
    }

    if (package->opcode != CPU_RESPONSE_INSTRUCTION) {
        LOG_ERROR("Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    instruction_t* instruction = malloc(sizeof(instruction_t));
    buffer_read(package->buffer, &instruction->instruction, sizeof(cod_instruction));

    uint32_t operand_count;
    buffer_read(package->buffer, &operand_count, sizeof(uint32_t));
    //TODO creo que deberia agregar a la estructura de instruccion la cantidad de operandos, no se si es necesario, pero lo dejo por las dudas
    instruction->operands = list_create();
    for (uint32_t i = 0; i < operand_count; i++) {
        uint32_t operand;
        buffer_read(package->buffer, &operand, sizeof(uint32_t));
        list_add(instruction->operands, (void*)operand);
    }

    package_destroy(package);
    return instruction;
}