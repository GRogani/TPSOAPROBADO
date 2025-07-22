#include "main.h"

_Atomic int32_t pid = 0, pc = 0;

int main(int argc, char *argv[])
{
    init_list_and_mutex();

    if (argc < 2)
    {
        argv[1] = "cpu.config";
    }

    if (argc < 3)
    {
        argv[2] = "cpu.log";
    }

    t_config *config_file = init_config(argv[1]);
    t_cpu_config config_cpu = init_cpu_config(config_file);
    init_logger(argv[2], "CPU", config_cpu.LOG_LEVEL);

    MMUConfig mmu_config = load_mmu_config(argv);
    TLBConfig tlb_config = load_tlb_config(argv);
    CacheConfig cache_config = load_cache_config(config_file);
    mmu_init(&mmu_config, &tlb_config, &cache_config);

    int memory_socket = -1;
    int kernel_dispatch_socket = -1;
    int kernel_interrupt_socket = -1;

    create_connections(config_cpu, &memory_socket, &kernel_dispatch_socket, &kernel_interrupt_socket);

    interrupt_args_t thread_args = {kernel_interrupt_socket, &pid, &pc, memory_socket};

    pthread_t interrupt_listener_thread;
    pthread_create(&interrupt_listener_thread, NULL, interrupt_listener, &thread_args);

    while (1)
    {
        recv_dispatch(kernel_dispatch_socket, &pid, &pc);
        
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
            destroy_package(instruction_package);

            bool should_preempt = execution(instruction, memory_socket, kernel_dispatch_socket, &pid, &pc);

            cleanup_instruction(instruction);

            bool should_interrupt = interrupt_handler(&thread_args);

            if (should_preempt || should_interrupt)
            {
                LOG_INFO("Preemption or interruption detected, breaking the instruction cycle.");
                pid = -1; 
                pc = -1;
                unlock_cpu_mutex();
                break;
            }
        };
    }

    LOG_INFO("Closing connections...");

    // Señalar al thread de interrupciones que debe terminar
    signal_interrupt_thread_exit();

    // Cerrar el socket de interrupciones para forzar que recv_package retorne NULL
    close(kernel_interrupt_socket);

    // Esperar a que el thread termine
    pthread_join(interrupt_listener_thread, NULL);
    LOG_INFO("Interrupt listener thread joined successfully");

    close(memory_socket);
    close(kernel_dispatch_socket);

    destroy_list_and_mutex();
    log_destroy(get_logger());
    config_destroy(config_file);

    return 0;
}

void cleanup_instruction(t_instruction *instruction)
{
    if (instruction == NULL)
        return;

    if (instruction->operand_string != NULL)
        free(instruction->operand_string);

    free(instruction);
}
