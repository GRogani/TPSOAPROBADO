#include "main.h"

sem_t cpu_mutex;

int main(int argc, char* argv[]) 
{
    sem_init(&cpu_mutex, 0, 1);

    t_config* config_file = init_config("cpu.config");
    t_cpu_config config_cpu = init_cpu_config(config_file);
    init_logger("cpu.log", "CPU", config_cpu.LOG_LEVEL);

    int fd_memory = -1;
    int fd_kernel_dispatch = -1;
    int fd_kernel_interrupt = -1;

    create_connections(config_cpu, &fd_memory, &fd_kernel_dispatch, &fd_kernel_interrupt);

    log_info(get_logger(), "Connections established successfully!");

    t_package* kernel_package = NULL;
    t_package* instruction_package = NULL;
    uint32_t pid, pc;
    instruction_t* instruction = NULL;

    interrupt_args_t thread_args = {fd_kernel_interrupt, &pid, &pc};
    pthread_t interrupt_thread;
    pthread_create(&interrupt_thread, NULL, interrupt_handler, &thread_args);

    while(1)
    {
        sem_wait(&cpu_mutex);

        kernel_package = receive_PID_PC_Package(fd_kernel_dispatch, &pid, &pc);
        if(kernel_package == NULL) break; 

        instruction_package = fetch(fd_memory, pid, pc);
        instruction = decode(instruction_package);
        execute(instruction, instruction_package, fd_memory, fd_kernel_dispatch, &pc);

        sem_post(&cpu_mutex);

        package_destroy(kernel_package);
        package_destroy(instruction_package);
        free(instruction->operand_string);
        free(instruction);
    }
    
    log_info(get_logger(), "Closing connections...");
    pthread_join(interrupt_thread, NULL);
    sem_destroy(&cpu_mutex);
    log_destroy(get_logger());
    close(fd_memory);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    config_destroy(config_file);

    return 0;
}

