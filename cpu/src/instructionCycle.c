#include "instructionCycle.h"

t_package *fetch(int socket, uint32_t PID, uint32_t PC)
{
    t_package *package;

    log_debug(get_logger(), "Fetching instruction for PID: %d, PC: %d", PID, PC);

    send_memory_get_instruction_request(socket, PID, PC);

    package = receive_instruction(socket);

    return package;
}

t_instruction *decode(t_package *package)
{
    t_instruction *instruction = safe_malloc(sizeof(t_instruction));

    char *instruction_string = read_memory_get_instruction_response(package);

    parse_instruction(instruction_string, instruction);

    free(instruction_string);

    return instruction;
}

void parse_instruction(char *instruction_string, t_instruction *instruction)
{
    char *saveptr;
    char *token = strdup(strtok_r(instruction_string, " ", &saveptr)); // hago que el token tenga su propia memoria liberable, asi puedo liberar el instruction_string antes
    if (token == NULL)
    {
        log_error(get_logger(), "Invalid instruction format");
        return;
    }

    instruction->instruction_code = string_to_instruction_code(token);

    switch (instruction->instruction_code)
    {
    case NOOP:
        break;
    case WRITE:
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &saveptr));
        instruction->operand_string = strdup(strtok_r(NULL, " ", &saveptr));
        instruction->operand_string_size = strlen(instruction->operand_string);
        break;
    case READ:
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &saveptr));
        instruction->operand_numeric2 = atoi(strtok_r(NULL, " ", &saveptr));
        break;
    case GOTO:
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &saveptr));
        break;
    case IO:
        instruction->operand_string = strdup(strtok_r(NULL, " ", &saveptr)); // 1. device_name
        instruction->operand_string_size = strlen(instruction->operand_string);
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &saveptr)); // 2. sleep time
        break;
    case INIT_PROC:
        instruction->operand_string = strdup(strtok_r(NULL, " ", &saveptr)); // 1. pseudocode file
        instruction->operand_string_size = strlen(instruction->operand_string);
        instruction->operand_numeric1 = atoi(strtok_r(NULL, " ", &saveptr)); // 2. memory space
        break;
    case DUMP_PROCESS:
    case EXIT:
    {
        break; // dump memory and exit syscalls has nothing to do with operands
    }
    default:
        log_error(get_logger(), "Unknown instruction code: %d", instruction->instruction_code);
        break;
    }
}

int execute(t_instruction *instruction, int socket_memory, int socket_dispatch, uint32_t *pid, uint32_t *PC)
{
    switch (instruction->instruction_code)
    {
    case NOOP:
    {
        // No operation
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
            log_info(get_logger(), "Data read from memory: %s", data);
            free(data);
        }
        else
        {
            log_error(get_logger(), "Failed to read data from memory");
            return -1;
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
        log_debug(get_logger(), "Executing IO operation, sending syscall to kernel");
        (*PC)++;
        t_cpu_syscall *syscall_req = safe_malloc(sizeof(t_cpu_syscall));
        syscall_req->syscall_type = SYSCALL_IO;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        syscall_req->params.io.device_name = instruction->operand_string;
        syscall_req->params.io.sleep_time = instruction->operand_numeric1;
        send_cpu_syscall_request(socket_dispatch, syscall_req);
        destroy_cpu_syscall(syscall_req);
        return 1;
    }
    case INIT_PROC:
    {
        log_debug(get_logger(), "Executing INIT_PROC, sending syscall to kernel");
        (*PC)++;
        t_cpu_syscall *syscall_req = safe_malloc(sizeof(t_cpu_syscall));
        syscall_req->syscall_type = SYSCALL_INIT_PROC;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        syscall_req->params.init_proc.pseudocode_file = instruction->operand_string;
        syscall_req->params.init_proc.memory_space = instruction->operand_numeric1;
        send_cpu_syscall_request(socket_dispatch, syscall_req);
        destroy_cpu_syscall(syscall_req);

        // wait for response from kernel to continue execution
        t_package *package = recv_package(socket_dispatch);
        int success = read_cpu_syscall_response(package);
        if (!success)
        {
            log_error(get_logger(), "Failed to initialize process for PID %d", pid);
            package_destroy(package);
            return -1;
        }
        break;
    }
    case DUMP_PROCESS:
    {
        log_debug(get_logger(), "Executing DUMP_PROCESS, sending syscall to kernel");
        (*PC)++;
        t_cpu_syscall *syscall_req = safe_malloc(sizeof(t_cpu_syscall));
        syscall_req->syscall_type = SYSCALL_DUMP_PROCESS;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        send_cpu_syscall_request(socket_dispatch, syscall_req);
        destroy_cpu_syscall(syscall_req);
        return 1;
    }
    case EXIT:
    {
        log_debug(get_logger(), "Executing EXIT, sending syscall to kernel");
        (*PC)++;
        t_cpu_syscall *syscall_req = safe_malloc(sizeof(t_cpu_syscall));
        syscall_req->syscall_type = SYSCALL_EXIT;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        send_cpu_syscall_request(socket_dispatch, syscall_req);
        destroy_cpu_syscall(syscall_req);
        return 1;
    }
    default:
    {
        log_error(get_logger(), "Unknown instruction: %d", instruction->instruction_code);
        return -1;
    }
    }
    return 0;
}

int check_interrupt(int socket_interrupt, t_package *package, uint32_t *pid_on_execute, uint32_t *pc_on_execute)
{

    if (package == NULL)
    {
        log_error(get_logger(), "Disconnecting interrupt...");
        return -1;
    }

    if (package->opcode == CPU_INTERRUPT)
    {
        int pid_received = read_cpu_interrupt_request(package);

        if (pid_received == *pid_on_execute)
        {
            log_info(get_logger(), "Interrupt for PID %d executed", pid_received);
        }
        else
        {
            log_info(get_logger(), "Interrupt for PID %d received, but not executing", pid_received);
        }

        send_cpu_interrupt_response(socket_interrupt, pid_received, pc_on_execute);
    }
    else
    {
        log_error(get_logger(), "Received unexpected opcode on interrupt connection: %s", opcode_to_string(package->opcode));
    }

    return 0;
}

void *interrupt_listener(void *socket)
{
    while (1)
    {
        t_package *package = recv_package(*(int *)socket);
        if (package == NULL)
        {
            log_error(get_logger(), "Disconnecting interrupt listener...");
            return NULL;
        }
        lock_interrupt_list();
        log_info(get_logger(), "Received interrupt package with opcode: %s", opcode_to_string(package->opcode));
        log_debug(get_logger(), "Adding interrupt package with opcode: %s", opcode_to_string(package->opcode));
        add_interrupt(package);
        unlock_interrupt_list();
        signal_interrupt();
    }
}

void *interrupt_handler(void *thread_args)
{
    interrupt_args_t *args = (interrupt_args_t *)thread_args;
    while (1)
    {
        wait_interrupt();
        lock_interrupt_list();
        while (interrupt_count() > 0)
        {
            log_debug(get_logger(), "Checking interrupts");
            t_package *package = get_last_interrupt(interrupt_count());
            lock_cpu_mutex();
            check_interrupt(args->socket_interrupt, package, args->pid, args->pc);
            unlock_cpu_mutex();
        }
        unlock_interrupt_list();
    }

    return NULL;
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

    return (uint32_t)(physic_dir * size_pag + offset);
}