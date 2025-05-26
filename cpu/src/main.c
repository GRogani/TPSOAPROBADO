#include "main.h"

int main(int argc, char* argv[]) 
{
    t_config* config_file = init_config("cpu.config");
    t_cpu_config config_cpu = init_cpu_config(config_file);
    init_logger("cpu.log", "CPU", config_cpu.LOG_LEVEL);

    int fd_memory = -1;
    int fd_kernel_dispatch = -1;
    int fd_kernel_interrupt = -1;

    while( (fd_memory < 0) && (fd_kernel_dispatch < 0) && (fd_kernel_interrupt < 0) )
    {
        create_connections(config_cpu, &fd_memory, &fd_kernel_dispatch, &fd_kernel_interrupt);
        if( (fd_memory * fd_kernel_dispatch * fd_kernel_interrupt) < 0)
        {
            log_info(get_logger(), "Retrying connections...");
            sleep(3);
        }
    }

    log_info(get_logger(), "Connections established successfully!");

    while (1)
    {
       t_package* package_pid_pc = receive_PID_PC_Package(fd_kernel_dispatch);
       int pid = buffer_read_uint32(package_pid_pc->buffer);
       int pc = buffer_read_uint32(package_pid_pc->buffer);       
       int interrupt =  0;
       while(interrupt == 0){
       t_package* instruction_package = fetch(fd_memory, pc);
       instruction_t* instruction = decode(instruction_package);
           execute(instruction, instruction_package, fd_memory, fd_kernel_dispatch, &pc);
           interrupt = check_interrupt(fd_kernel_interrupt, pid, pc);
           package_destroy(instruction_package);
           free(instruction);
       }

        package_destroy(package_pid_pc);
    }
    

    close(fd_memory);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    config_destroy(config_file);
    return 0;
}

void create_connections(t_cpu_config config_cpu, int* fd_memory, int* fd_kernel_dispatch, int* fd_kernel_interrupt)
{
    if(*fd_memory < 0)
        *fd_memory = create_connection(config_cpu.PUERTO_MEMORIA, config_cpu.IP_MEMORIA);
    if(*fd_kernel_dispatch < 0)    
        *fd_kernel_dispatch = create_connection(config_cpu.PUERTO_KERNEL_DISPATCH, config_cpu.IP_KERNEL);
    if(*fd_kernel_interrupt < 0)
        *fd_kernel_interrupt = create_connection(config_cpu.PUERTO_KERNEL_INTERRUPT, config_cpu.IP_KERNEL);
}
