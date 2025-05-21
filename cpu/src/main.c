#include "main.h"

int main(int argc, char* argv[]) 
{
    t_config* config_cpu = init_config("cpu.config");

    t_log_level level = log_level_from_string(config_get_string_value(config_cpu,"LOG_LEVEL"));
    init_logger("cpu.log", "CPU", level);

    int fd_memory = create_connection(config_get_string_value(config_cpu, "PUERTO_MEMORIA"), config_get_string_value(config_cpu, "IP_MEMORIA"));
    int fd_kernel_dispatch = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH"), config_get_string_value(config_cpu, "IP_KERNEL"));
    int fd_kernel_interrupt = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT"), config_get_string_value(config_cpu, "IP_KERNEL"));
    
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
    config_destroy(config_cpu);
    return 0;
}
