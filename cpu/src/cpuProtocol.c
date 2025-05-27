#include "cpuProtocol.h"

t_package* receive_PID_PC_Package(int socket_dispatch_kernel) {
    t_package* package = recv_package(socket_dispatch_kernel);
    if (package == NULL) {
       log_error(get_logger(), "Failed to receive package");
        return NULL;
    }

    if (package->opcode != PID_PC_PACKAGE) {
       log_error(get_logger(), "Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    return package;
}

void request_instruction(int socket, int PC) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, PC);
    t_package* package = package_create(GET_INSTRUCTION, buffer);
    send_package(socket, package);
    package_destroy(package);
}

t_package* receive_instruction(int socket) {
    t_package* package = recv_package(socket);
    if (package == NULL) {
        log_error(get_logger(), "Failed to receive package");
        return NULL;
    }

    if (package->opcode != GET_INSTRUCTION) {
        log_error(get_logger(), "Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    return package;
}

void write_memory_request(int socket_memory, uint32_t direccion_fisica, char* valor_write) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + strlen(valor_write) + 1);
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_string(buffer, strlen(valor_write), valor_write);
    t_package* package = package_create(WRITE_MEMORY, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

void read_memory_request(int socket_memory, uint32_t direccion_fisica, uint32_t size) {
    t_buffer* buffer = buffer_create( 2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_uint32(buffer, size);
    t_package* package = package_create(READ_MEMORY, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

char* read_memory_response(int socket_memory) {
    t_package* package = recv_package(socket_memory);
    if (package == NULL) {
       log_error(get_logger(), "Failed to receive package");
        return NULL;
    }

    if (package->opcode != READ_MEMORY) {
       log_error(get_logger(), "Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    char* data = buffer_read_string(package->buffer, package->buffer->stream_size);

    package_destroy(package);
    return data;
}

t_package* recv_interrupt_package(int socket_interrupt) {
OPCODE opcode;
    uint32_t buffer_stream_size;

    // recibir opcode
    // aca ya se puede validar si vale la pena seguir solo con el opcode
    if (recv(socket, &opcode, sizeof(OPCODE), MSG_DONTWAIT) <= 0) { 
        return NULL; 
    }

    // recibir el buffer size
    // quiza es muy grande se puede validar eso
    if (recv(socket, &buffer_stream_size, sizeof(uint32_t), MSG_DONTWAIT) <= 0) {
        return NULL; 
    }

    // hago espacio para buffer serializado
    void* buffer_stream_data = safe_malloc(buffer_stream_size);

    // guardo el buffer stream de data serializado / caso error libero memoria
    if (recv(socket, buffer_stream_data, buffer_stream_size, MSG_DONTWAIT) <= 0) {
        free(buffer_stream_data); 
        return NULL;
    }

    t_buffer* buffer = buffer_create(buffer_stream_size); // Crea un buffer con el tamaño recibido
    buffer_add(buffer, buffer_stream_data, buffer_stream_size); // Agrega los datos serializados al buffer

    t_package* package = package_create(opcode, buffer);

    free(buffer_stream_data); // liberar memoria de los datos serializados

    return package;

}