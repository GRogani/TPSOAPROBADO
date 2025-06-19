#include "main.h"

int main(int argc, char* argv[]) 
{
    init_list_and_mutex();

    t_config* config_file = init_config("cpu.config");
    t_cpu_config config_cpu = init_cpu_config(config_file);

    MMUConfig mmu_config = load_mmu_config();
    TLBConfig tlb_config = load_tlb_config();
    CacheConfig cache_config = load_cache_config(config_file);
    mmu_init(&mmu_config, &tlb_config, &cache_config);
    
    init_logger("cpu.log", "CPU", config_cpu.LOG_LEVEL);

    int memory_socket = -1;
    int kernel_dispatch_socket = -1;
    int kernel_interrupt_socket = -1;

    create_connections(config_cpu, &memory_socket, &kernel_dispatch_socket, &kernel_interrupt_socket);

    t_package* kernel_package = NULL;
    uint32_t pid, pc;
    t_instruction* instruction = NULL;

    interrupt_args_t thread_args = {kernel_interrupt_socket, &pid, &pc};
    pthread_t interrupt_handler_thread;
    pthread_t interrupt_listener_thread;
    pthread_create(&interrupt_listener_thread, NULL, interrupt_listener, &kernel_interrupt_socket);
    pthread_create(&interrupt_handler_thread, NULL, interrupt_handler, &thread_args);

    while (1)
    {   
        lock_cpu_mutex();
        if(interrupt_count() > 0)
        {
            unlock_cpu_mutex();
            break;
        }
        unlock_cpu_mutex();
        kernel_package = receive_PID_PC_Package(kernel_dispatch_socket, &pid, &pc);
        if(kernel_package == NULL) break;
        package_destroy(kernel_package);

        int syscall = 0;
        while (syscall != 1)
        {
            lock_cpu_mutex();

            t_package *instruction_package = fetch(memory_socket, pid, pc);
            if (instruction_package == NULL)
            {
                unlock_cpu_mutex();
                break;
            }

                t_instruction * instruction = decode(instruction_package);
                
                if (instruction == NULL)
                {
                    LOG_ERROR("Decoding error");
                    unlock_cpu_mutex();
                    break;
                }
                package_destroy(instruction_package);

                syscall = execute(instruction, memory_socket, kernel_dispatch_socket, &pid, &pc);

                unlock_cpu_mutex();
        };

        if (syscall == -1) 
            LOG_ERROR("Execution error");

        
        cleanup_instruction(instruction);
        
    }
    
    LOG_INFO("Closing connections...");

    pthread_join(interrupt_listener_thread, NULL);
    pthread_join(interrupt_handler_thread, NULL);

    close(memory_socket);
    close(kernel_dispatch_socket);
    close(kernel_interrupt_socket);

    destroy_list_and_mutex();
    log_destroy(get_logger());
    config_destroy(config_file);

    return 0;
}

void cleanup_instruction(t_instruction* instruction) 
{
    if (instruction != NULL) {
        if (instruction->operand_string_size > 0)
            free(instruction->operand_string);
        free(instruction);
    }
}

