#include "main.h"

int main(int argc, char *argv[])
{
    init_list_and_mutex();
    t_config *config_file = init_config("cpu.config");
    t_cpu_config config_cpu = init_cpu_config(config_file);
    init_logger("cpu.log", "CPU", config_cpu.LOG_LEVEL);

    int memory_socket = -1;
    int kernel_dispatch_socket = -1;
    int kernel_interrupt_socket = -1;

    create_connections(config_cpu, &memory_socket, &kernel_dispatch_socket, &kernel_interrupt_socket);

    t_package *kernel_package = NULL;
    uint32_t pid, pc;

    interrupt_args_t thread_args = {kernel_interrupt_socket, &pid, &pc};
    
    pthread_t interrupt_listener_thread;
    pthread_create(&interrupt_listener_thread, NULL, interrupt_listener, &thread_args);

    while (1)
    {
        kernel_package = receive_PID_PC_Package(kernel_dispatch_socket, &pid, &pc);
        if (kernel_package == NULL)
        {
            LOG_INFO("Disconneted from Kernel Dispatch");
            break;
        }

        while (1)
        {

            t_package *instruction_package = fetch(memory_socket, pid, pc);
            if (instruction_package == NULL)
            {
                LOG_INFO("Disconnected from Memory");
                unlock_cpu_mutex();
                break;
            }

            t_instruction *instruction = decode(instruction_package);

            if (instruction == NULL)
            {
                LOG_ERROR("Decoding error");
                unlock_cpu_mutex();
                break;
            }
            package_destroy(instruction_package);

            bool should_preempt = execute(instruction, memory_socket, kernel_dispatch_socket, &pid, &pc);

            cleanup_instruction(instruction);

            bool should_interrupt = interrupt_handler(&thread_args);

            if (should_preempt || should_interrupt)
            {
                LOG_DEBUG("Preemption or interruption detected, breaking the instruction cycle.");
                // limpiamos PC y PID
                pid = 93847593; // un valor que es imposible que sea un PID real, lo consideramos basura
                pc = 0;
                unlock_cpu_mutex();
                break;
            }
        };
    }

    LOG_INFO("Closing connections...");

    pthread_join(interrupt_listener_thread, NULL);

    close(memory_socket);
    close(kernel_dispatch_socket);
    close(kernel_interrupt_socket);

    destroy_list_and_mutex();
    log_destroy(get_logger());
    config_destroy(config_file);

    return 0;
}

void cleanup_instruction(t_instruction *instruction)
{
    if (instruction == NULL)
        return;

    if (instruction->operand_string == NULL)
        return;

    free(instruction->operand_string);
    free(instruction);
}
