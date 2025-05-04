#include "cpuProtocol.h"

int receive_PID(int socket_dispatch_kernel) {
    t_package* package = recv_package(socket);
    if (package == NULL) {
        log_error(get_logger() ,"Failed to receive package");
        return NULL;
    }

    if (package->opcode != CPU_PID) {
       log_error(get_logger() ,"Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    uint32_t pid;
    package->buffer->offset = 0;
    pid = buffer_read_uint32(package->buffer);

    int result = malloc(sizeof(int));
    result = pid;

    package_destroy(package);
    return result;
}

int receive_PC(int socket_dispatch_kernel) {
    t_package* package = recv_package(socket);
    if (package == NULL) {
        LOG_ERROR("Failed to receive package");
        return NULL;
    }

    if (package->opcode != CPU_PC) {
        LOG_ERROR("Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    uint32_t pc;
    buffer_read(package->buffer, &pc, sizeof(uint32_t));

    int result = malloc(sizeof(int));
    result = pc;

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

void write_memory_request(int socket_memory, uint32_t direccion_fisica, char* valor_write) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + strlen(valor_write) + 1);
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_string(buffer, valor_write);
    t_package* package = package_create(CPU_WRITE_MEMORY_REQUEST, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

void read_memory_request(int socket_memory, uint32_t direccion_fisica, uint32_t size) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(uint32_t));
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_uint32(buffer, size);
    t_package* package = package_create(CPU_READ_MEMORY_REQUEST, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

char* read_memory_response(int socket_memory) {
    t_package* package = recv_package(socket_memory);
    if (package == NULL) {
        LOG_ERROR("Failed to receive package");
        return NULL;
    }

    if (package->opcode != CPU_READ_MEMORY_RESPONSE) {
        LOG_ERROR("Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    char* data = buffer_read_string(package->buffer, package->buffer->size);

    package_destroy(package);
    return data;
}