#include "main.h"

int main(int argc, char* argv[]) 
{
    t_config* config_cpu = init_config("cpu.config");

    t_log_level level = log_level_from_string(config_get_string_value(config_cpu,"LOG_LEVEL"));
    init_logger("cpu.log", "CPU", level);

    int fd_memory = create_connection(config_get_string_value(config_cpu, "PUERTO_MEMORIA"), config_get_string_value(config_cpu, "IP_MEMORIA"));
    int fd_kernel_dispatch = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH"), config_get_string_value(config_cpu, "IP_KERNEL_DISPATCH"));
    int fd_kernel_interrupt = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT"), config_get_string_value(config_cpu, "IP_KERNEL_INTERRUPT"));


    close(fd_memory);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    config_destroy(config_cpu);
    return 0;
}
