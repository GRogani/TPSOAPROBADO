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