#include "instructionCycle.h"

t_package *fetch(int socket, uint32_t PID, uint32_t PC)
{
    t_package *package;

    LOG_INFO("Fetching instruction for PID: %d, PC: %d", PID, PC);

    send_fetch_package(socket, PID, PC);

    package = receive_instruction(socket);

    return package;
}

t_instruction *decode(t_package *package)
{
    t_instruction *instruction = safe_calloc(1, sizeof(t_instruction));

    char *instruction_string = read_instruction_package(package); 

    parse_instruction(instruction_string, instruction);

    free(instruction_string);

    return instruction;
}

void parse_instruction(char *instruction_string, t_instruction *instruction)
{
    char *already_parsed;
    char *instruction_code = strtok_r(instruction_string, " ", &already_parsed); // con el instruction_code no hace falta
    if (instruction_code == NULL)
    {
        LOG_ERROR("Invalid instruction format");
        return;
    }

    instruction->instruction_code = string_to_instruction_code(instruction_code);

    switch (instruction->instruction_code)
    {
    case DUMP_PROCESS:
    case EXIT:
    case NOOP:
        break;
    case WRITE:
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &already_parsed));
        instruction->operand_string = strdup(strtok_r(NULL, " ", &already_parsed));
        instruction->operand_string_size = strlen(instruction->operand_string);
        break;
    case READ:
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &already_parsed));
        instruction->operand_numeric2 = atoi(strtok_r(NULL, " ", &already_parsed));
        break;
    case GOTO:
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &already_parsed));
        break;
    case IO:
        instruction->operand_string = strdup(strtok_r(NULL, " ", &already_parsed)); // 1. device_name
        instruction->operand_string_size = strlen(instruction->operand_string);
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &already_parsed)); // 2. sleep time
        break;
    case INIT_PROC:
        instruction->operand_string = strdup(strtok_r(NULL, " ", &already_parsed)); // 1. pseudocode file
        instruction->operand_string_size = strlen(instruction->operand_string);
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &already_parsed)); // 2. memory space
        break;
    default:
        LOG_ERROR("Unknown instruction code: %d", instruction->instruction_code);
        break;
    }
}
bool execute(t_instruction *instruction, int socket_memory, int socket_dispatch, uint32_t *pid, uint32_t *PC)

{
    switch (instruction->instruction_code)
    {
    case NOOP:
    {
        // No operation
        (*PC)++;
        break;
    }
    case WRITE:
    {
        uint32_t logic_dir_write = instruction->operand_numeric1;
        char *valor_write = instruction->operand_string;
        uint32_t physic_dir_write = MMU(logic_dir_write);
        write_memory_request(socket_memory, physic_dir_write, valor_write);
        (*PC)++;
        break;
    }
    case READ:
    {
        uint32_t logic_dir_read = instruction->operand_numeric1;
        uint32_t size = instruction->operand_numeric2;
        uint32_t physic_dir_read = MMU(logic_dir_read);
        read_memory_request(socket_memory, physic_dir_read, size);
        char *data = read_memory_response(socket_memory);
        if (data != NULL)
        {
            LOG_INFO("Data read from memory: %s", data);
            free(data);
        }
        else
        {
            LOG_INFO("Failed to read data from memory");
            return true;
        }
        (*PC)++;
        break;
    }
    case GOTO:
    {
        *PC = instruction->operand_numeric1;
        break;
    }
    case IO:
    {
        LOG_INFO("Executing IO operation, sending syscall to kernel");
        (*PC)++;
        syscall_package_data *syscall_req = safe_malloc(sizeof(syscall_package_data));
        syscall_req->syscall_type = SYSCALL_IO;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        syscall_req->params.io.device_name = strdup(instruction->operand_string);
        syscall_req->params.io.sleep_time = instruction->operand_numeric1;
        send_syscall_package(socket_dispatch, syscall_req);
        destroy_syscall_package(syscall_req);
        return true;
    }
    case INIT_PROC:
    {
        LOG_INFO("Executing INIT_PROC, sending syscall to kernel");
        (*PC)++;
        syscall_package_data *syscall_req = safe_malloc(sizeof(syscall_package_data));
        syscall_req->syscall_type = SYSCALL_INIT_PROC;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        syscall_req->params.init_proc.pseudocode_file = instruction->operand_string;
        syscall_req->params.init_proc.memory_space = instruction->operand_numeric1;
        send_syscall_package(socket_dispatch, syscall_req);
        destroy_syscall_package(syscall_req);

        // wait for response from kernel to continue execution
        t_package *package = recv_package(socket_dispatch);
        if (package->opcode != CONFIRMATION)
        {
            LOG_ERROR("Failed to receive confirmation package from kernel for PID %d", *pid);
            return true; // should preempt due an issue
        }
        int success = read_confirmation_package(package);
        if (!success)
        {
            LOG_ERROR("Failed to initialize process for PID %d", *pid);
            destroy_package(package);
            return true;
        }
        break;
    }
    case DUMP_PROCESS:
    {
        LOG_INFO("Executing DUMP_PROCESS, sending syscall to kernel");
        (*PC)++;
        syscall_package_data *syscall_req = safe_malloc(sizeof(syscall_package_data));
        syscall_req->syscall_type = SYSCALL_DUMP_PROCESS;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        send_syscall_package(socket_dispatch, syscall_req);
        destroy_syscall_package(syscall_req);
        return true;
    }
    case EXIT:
    {
        LOG_INFO("Executing EXIT, sending syscall to kernel");
        (*PC)++;
        syscall_package_data *syscall_req = safe_malloc(sizeof(syscall_package_data));
        syscall_req->syscall_type = SYSCALL_EXIT;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        send_syscall_package(socket_dispatch, syscall_req);
        destroy_syscall_package(syscall_req);
        return true;
    }
    default:
    {
        LOG_WARNING("Unknown instruction: %d", instruction->instruction_code);
        return true;
    }
    }
    return false;
}

void check_interrupt(int socket_interrupt, t_package *package, uint32_t *pid_on_execute, uint32_t *pc_on_execute)
{
    if (package->opcode == INTERRUPT)
    {
        int pid_received = read_interrupt_package(package);

        if (pid_received == *pid_on_execute)
        {

            send_cpu_context_package(socket_interrupt, pid_received, *pc_on_execute, 0);

            LOG_INFO("Interrupt for PID %d executed", pid_received);

            return;
        }
        else
        {
            LOG_ERROR("Interrupt for PID %d received, but not executing", pid_received);

            send_cpu_context_package(socket_interrupt, pid_received, *pc_on_execute, 1); // si no fué interrumpido el mismo proceso, significa que no estaba ejecutando la CPU. el kernel necesita saber eso, para ver si debe mover el proceso a READY porque lo desalojó correctamente, o se autodesalojó por una syscall. (la syscall pasa automaticamente el proceso a su nuevo estado y guarda el PCB)
        }
    }
    else
    {
        LOG_WARNING("Received unexpected opcode on interrupt connection: %s", opcode_to_string(package->opcode));
    }

}

void *interrupt_listener(void *args)
{
    interrupt_args_t* thread_args = (interrupt_args_t *)args;

    while (!should_interrupt_thread_exit())
    {
        t_package *package = recv_package(thread_args->socket_interrupt);
        if (package == NULL)
        {
            LOG_INFO("Disconnected from Kernel Interrupt - interrupt_listener exiting");
            break;
        }
        
        lock_interrupt_list();
        LOG_INFO("Received interrupt package with opcode: %s", opcode_to_string(package->opcode));
        add_interrupt(package);
        unlock_interrupt_list();

        lock_cpu_mutex();
        interrupt_handler(args);
        unlock_cpu_mutex();
    }
    
    LOG_INFO("Interrupt listener thread exiting cleanly");
    pthread_exit(NULL);
}

bool interrupt_handler(void *thread_args)
{
    LOG_INFO("Interrupt handler working...");
    interrupt_args_t *args = (interrupt_args_t *)thread_args;
    bool interrupted = false;

    lock_interrupt_list();
    while (interrupt_count() > 0)
    {
        interrupted = true;
        t_package *package = get_last_interrupt(interrupt_count());
        check_interrupt(args->socket_interrupt, package, args->pid, args->pc);
        destroy_package(package);
    }
    unlock_interrupt_list();

    LOG_INFO("Interrupt found: %s", interrupted ? "yes" : "no");

    return interrupted;
}

uint32_t MMU(uint32_t logic_dir)
{
    t_config *config_memoria = init_config("../memoria/memoria.config");
    uint32_t levels = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    uint32_t entrys_by_table = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    uint32_t size_pag = config_get_int_value(config_memoria, "TAM_PAGINA");

    uint32_t num_page = logic_dir / size_pag;
    uint32_t offset = logic_dir % size_pag;
    uint32_t physic_dir = 0;

    for (int actual_level = 1; actual_level <= levels; actual_level++)
    {
        uint32_t divisor = pow(entrys_by_table, levels - actual_level);
        uint32_t entry = (num_page / divisor) % entrys_by_table;
        physic_dir = physic_dir * entrys_by_table + entry;
    }

    uint32_t result = (uint32_t)(physic_dir * size_pag + offset);
    
    // Liberar la configuración para evitar memory leak
    config_destroy(config_memoria);
    
    return result;
}