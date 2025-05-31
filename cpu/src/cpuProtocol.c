#include "cpuProtocol.h"

t_package* receive_PID_PC_Package(int socket_dispatch_kernel, uint32_t* PID, uint32_t* PC) 
{
    extern sem_t cpu_mutex; // en main
    t_package* package;
    bool corrupted_package;

    do{
        LOG_DEBUG("Waiting for PID and PC package from kernel...");
        corrupted_package = false;
        package = recv_package(socket_dispatch_kernel);
        if (package == NULL) 
        {
            LOG_INFO("Disconnected from kernel"); // ESTE CASO ES DE DESCONECCION, OSEA ESPERABLE LOS OTROS NO
            return NULL;
        }
        else if (package->opcode != PID_PC_PACKAGE) 
        {
            corrupted_package = true;
            LOG_ERROR("Received package with unexpected opcode: %s", opcode_to_string(package->opcode) );
            package_destroy(package);
        }
        else if (package->buffer == NULL)
        {
            corrupted_package = true;
            LOG_WARNING("Received package with NULL buffer");
            package_destroy(package);
        }
        if (corrupted_package) 
        {
            LOG_INFO("Retrying to receive PID and PC package...");
        }
    } while (corrupted_package);

    sem_wait(&cpu_mutex);

    t_cpu_dispatch * cpu_dispatch = read_cpu_dispatch_request(package);
    *PID = cpu_dispatch->pid;
    *PC = cpu_dispatch->pc;
    destroy_cpu_dispatch(cpu_dispatch);
    
    sem_post(&cpu_mutex);

    return package;
}

t_package* receive_instruction(int socket) 
{
    t_package* package;
    bool corrupted_package;
    do{
        corrupted_package = false;
        package = recv_package(socket);

        if (package == NULL) {
            LOG_INFO("Disconnected from memory");
            return NULL;
        }
        else if (package->opcode != GET_INSTRUCTION) {
            corrupted_package = true;
            LOG_ERROR("Received package with unexpected opcode: %d", package->opcode);
            package_destroy(package);
        }
    }while(corrupted_package);

    return package;
}


// TODO: move these to DTOs
void write_memory_request(int socket_memory, uint32_t direccion_fisica, char* valor_write) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + strlen(valor_write) + 1);
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_string(buffer, strlen(valor_write), valor_write);
    t_package* package = package_create(WRITE_MEMORY, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

// TODO: move these to DTOs
void read_memory_request(int socket_memory, uint32_t direccion_fisica, uint32_t size) 
{
    t_buffer* buffer = buffer_create( 2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, direccion_fisica);
    buffer_add_uint32(buffer, size);
    t_package* package = package_create(READ_MEMORY, buffer);
    send_package(socket_memory, package);
    package_destroy(package);
}

// TODO: move these to DTOs
char* read_memory_response(int socket_memory) {
    t_package* package = recv_package(socket_memory);
    if (package == NULL) {
       LOG_ERROR("Failed to receive Read response from memory");
        return NULL;
    }

    if (package->opcode != READ_MEMORY) {
       LOG_ERROR("Received package with unexpected opcode: %d", package->opcode);
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
            LOG_INFO("Retrying connections...");
            sleep(3);
        }
    }

    LOG_INFO("Kernel Dispatch assigned to socket: %d", *fd_kernel_dispatch);
    LOG_INFO("Kernel Interrupt assigned to socket: %d", *fd_kernel_interrupt);
    LOG_INFO("Memory assigned to socket: %d", *fd_memory);
}