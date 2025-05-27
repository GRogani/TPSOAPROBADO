#include "cpuProtocol.h"

t_package* receive_PID_PC_Package(int socket_dispatch_kernel, uint32_t* PID, uint32_t* PC) 
{

    t_package* package = recv_package(socket_dispatch_kernel);

    if (package == NULL) {
       log_error(get_logger(), "Disconnected from kernel");
        return NULL;
    }

    if (package->opcode != PID_PC_PACKAGE) {
       log_error(get_logger(), "Received package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    package->buffer->offset = 0;
    *PID = buffer_read_uint32(package->buffer);
    *PC = buffer_read_uint32(package->buffer);

    return package;
}

void request_instruction(int socket, uint32_t PID, uint32_t PC) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t)*2);
    buffer_add_uint32(buffer, PID);
    buffer_add_uint32(buffer, PC);

    t_package* package = package_create(CPU_REQUEST_INSTRUCTION, buffer);

    send_package(socket, package);
    package_destroy(package);
}

t_package* receive_instruction(int socket) 
{
    t_package* package = recv_package(socket);

    if (package == NULL) {
        log_error(get_logger(), "Failed to receive instruction package");
        return NULL;
    }

    if (package->opcode != CPU_RESPONSE_INSTRUCTION) {
        log_error(get_logger(), "Received instruction package with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    return package;
}

void write_memory_request(int socket_memory, uint32_t direccion_fisica, char* valor_write) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + strlen(valor_write) + 1);
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_string(buffer, strlen(valor_write), valor_write);

    t_package* package = package_create(CPU_WRITE_MEMORY_REQUEST, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

void read_memory_request(int socket_memory, uint32_t direccion_fisica, uint32_t size) 
{
    t_buffer* buffer = buffer_create( 2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_uint32(buffer, size);

    t_package* package = package_create(CPU_READ_MEMORY_REQUEST, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

char* read_memory_response(int socket_memory) {
    t_package* package = recv_package(socket_memory);
    if (package == NULL) {
       log_error(get_logger(), "Failed to receive Read response from memory");
        return NULL;
    }

    if (package->opcode != CPU_READ_MEMORY_RESPONSE) {
       log_error(get_logger(), "Received memory response with unexpected opcode: %d", package->opcode);
        package_destroy(package);
        return NULL;
    }

    package->buffer->offset = 0;
    uint32_t bytes_read = 0;
    char* data = buffer_read_string(package->buffer, &bytes_read);

    package_destroy(package);
    
    return data;
}

void create_connections(t_cpu_config config_cpu, int* fd_memory, int* fd_kernel_dispatch, int* fd_kernel_interrupt)
{
    while( (*fd_memory < 0) && (*fd_kernel_dispatch < 0) && (*fd_kernel_interrupt < 0) )
    {
        if(*fd_memory < 0)
            *fd_memory = create_connection(config_cpu.PUERTO_MEMORIA, config_cpu.IP_MEMORIA);

        if(*fd_kernel_dispatch < 0)    
            *fd_kernel_dispatch = create_connection(config_cpu.PUERTO_KERNEL_DISPATCH, config_cpu.IP_KERNEL);

        if(*fd_kernel_interrupt < 0)
            *fd_kernel_interrupt = create_connection(config_cpu.PUERTO_KERNEL_INTERRUPT, config_cpu.IP_KERNEL);

        if( (*fd_memory < 0) && (*fd_kernel_dispatch < 0) && (*fd_kernel_interrupt < 0) )
        {
            log_info(get_logger(), "Retrying connections...");
            sleep(3);
        }
    }
}