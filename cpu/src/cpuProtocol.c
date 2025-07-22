#include "cpuProtocol.h"

t_package* recv_dispatch (int socket_dispatch_kernel, _Atomic int32_t* PID, _Atomic int32_t* PC) 
{
    if (socket_dispatch_kernel < 0)
        return NULL;

    t_package* package;
    bool corrupted_package;

    //do{
        LOG_INFO("Waiting PID & PC from kernel...");
        corrupted_package = false;
        package = recv_package(socket_dispatch_kernel);
        if (package == NULL) 
        {
            // ESTE CASO ES DE DESCONECCION, OSEA ESPERABLE LOS OTROS NO, muevo log al main
            return NULL;
        }
        else if (package->opcode != DISPATCH) 
        {
            corrupted_package = true;
            LOG_ERROR("Received package with unexpected opcode: %s", opcode_to_string(package->opcode) );
            destroy_package(package);
            return NULL;
        }
        else if (package->buffer == NULL)
        {
            corrupted_package = true;
            LOG_WARNING("Received package with NULL buffer");
            destroy_package(package);
            return NULL;
        }
        if (corrupted_package) 
        {
            //LOG_INFO("Retrying to receive PID and PC package...");
            return NULL;
        }
    //} while (corrupted_package);

    lock_cpu_mutex();

    dispatch_package_data  cpu_dispatch = read_dispatch_package(package);
    *PID = cpu_dispatch.pid;
    *PC = cpu_dispatch.pc;
    
    // The package needs to be destroyed after extracting the data
    destroy_package(package);

    return NULL; // Changed to return NULL since we've extracted the data and freed the package
}

t_package* receive_instruction(int socket) 
{
    t_package* package;
    //bool corrupted_package;
    //do{
        //corrupted_package = false;
        package = recv_package(socket);

        if (package == NULL) {
            //disconnection log en main
            return NULL;
        }
        else if (package->opcode != INSTRUCTION) {
            //corrupted_package = true;
            LOG_ERROR("Received package with unexpected opcode: %s", opcode_to_string(package->opcode) );
            destroy_package(package);
            return NULL;
        }
    //}while(corrupted_package);

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