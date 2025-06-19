#include "instructionCycle.h"

extern CacheConfig *g_cache_config;
extern TLBConfig *g_tlb_config;
extern MMUConfig *g_mmu_config;

t_package *fetch(int socket, uint32_t PID, uint32_t PC)
{
    t_package *package;

    LOG_DEBUG("Fetching instruction for PID: %d, PC: %d", PID, PC);

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
        if (g_cache_config->entry_count > 0)
        {
            // If cache is enabled, we need to write to cache first
            CacheEntry *cache_entry = cache_find_entry(logic_dir_write);
            if (cache_entry == NULL)
            {
                // Cache miss, load the page into cache
                cache_entry = cache_load_page(logic_dir_write, &socket_memory);
   
            }
            // Write to cache
            memcpy(cache_entry->content, valor_write, instruction->operand_string_size);
            cache_entry->modified_bit = true;
            (*PC)++;
            break;
        }
        else if(g_tlb_config->entry_count > 0)
        {
        uint32_t physic_dir_write = mmu_translate_address(logic_dir_write);
        t_memory_write_request *write_req = create_memory_write_request(physic_dir_write, instruction->operand_string_size, valor_write);
        send_memory_write_request(socket_memory, write_req);
        destroy_memory_write_request(write_req);
        (*PC)++;
        break;
        }
        uint32_t page_number = floor(logic_dir_write / g_mmu_config->page_size);
        uint32_t offset = logic_dir_write % g_mmu_config->page_size;
        uint32_t frame_number = mmu_perform_page_walk(page_number);
        uint32_t physic_dir_write = (frame_number * g_mmu_config->page_size) + offset;
        t_memory_write_request *write_req = create_memory_write_request(physic_dir_write, instruction->operand_string_size, valor_write);
        send_memory_write_request(socket_memory, write_req);
        destroy_memory_write_request(write_req);
        (*PC)++;
        break;
    }
    case READ:
    {
        uint32_t logic_dir_read = instruction->operand_numeric1;
        uint32_t size = instruction->operand_numeric2;
        if (g_cache_config->entry_count > 0)
        {
            // If cache is enabled, we need to read from cache first
            CacheEntry *cache_entry = cache_find_entry(logic_dir_read);
            if (cache_entry == NULL)
            {
                // Cache miss, load the page into cache
                cache_entry = cache_load_page(logic_dir_read, &socket_memory);
            }
            // Read from cache
            char *data = malloc(size);
            memcpy(data, cache_entry->content, size);
            LOG_INFO("Data read from cache: %s", data);
            free(data);
            (*PC)++;
            break;
        }
        else if(g_tlb_config->entry_count > 0)
        {
            uint32_t physic_dir_read = mmu_translate_address(logic_dir_read);
            t_memory_read_request* request = create_memory_read_request(physic_dir_read, size);
            send_memory_read_request(socket_memory, request);
            destroy_memory_read_request(request);

            t_package* package = recv_package(socket_memory);
            if (package == NULL || package->opcode != READ_MEMORY)
            {
                LOG_INFO("Failed to read data from memory");
                if(package) package_destroy(package);
                return -1;
            }

            t_memory_read_response* response = read_memory_read_response(package);
            package_destroy(package);

            if (response->data != NULL)
            {
                LOG_INFO("Data read from memory: %s", response->data);
                destroy_memory_read_response(response);
            }
            else
            {
                LOG_INFO("Failed to read data from memory");
                destroy_memory_read_response(response);
                return -1;
            }
            (*PC)++;
            break;
        }
        uint32_t page_number = floor(logic_dir_read / g_mmu_config->page_size);
        uint32_t offset = logic_dir_read % g_mmu_config->page_size;
        uint32_t frame_number = mmu_perform_page_walk(page_number);
        uint32_t physic_dir_read  = (frame_number * g_mmu_config->page_size) + offset;
        t_memory_read_request* request = create_memory_read_request(physic_dir_read, size);
        send_memory_read_request(socket_memory, request);
        destroy_memory_read_request(request);
        
        t_package* package = recv_package(socket_memory);
        if (package == NULL || package->opcode != READ_MEMORY)
        {
            LOG_INFO("Failed to read data from memory");
            if(package) package_destroy(package);
            return -1;
        }

        t_memory_read_response* response = read_memory_read_response(package);
        package_destroy(package);

        if (response->data != NULL)
        {
            LOG_INFO("Data read from memory: %s", response->data);
            destroy_memory_read_response(response);
        }
        else
        {
            LOG_INFO("Failed to read data from memory");
            destroy_memory_read_response(response);
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
        LOG_DEBUG("Executing IO operation, sending syscall to kernel");
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
        LOG_DEBUG("Executing INIT_PROC, sending syscall to kernel");
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
        bool success = read_cpu_syscall_response(package);
        if (!success)
        {
            log_error(get_logger(), "Failed to initialize process for PID %u", *pid);
            package_destroy(package);
            return -1;
        }
        break;
    }
    case DUMP_PROCESS:
    {
        LOG_DEBUG("Executing DUMP_PROCESS, sending syscall to kernel");
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
        LOG_DEBUG("Executing EXIT, sending syscall to kernel");
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
        LOG_WARNING("Unknown instruction: %d", instruction->instruction_code);
        return -1;
    }
    }
    return 0;
}

int check_interrupt(int socket_interrupt, t_package *package, uint32_t *pid_on_execute, uint32_t *pc_on_execute, int socket_memory)
{

    if (package == NULL)
    {
        LOG_INFO("Disconnecting interrupt...");
        return -1;
    }

    if (package->opcode == CPU_INTERRUPT)
    {
        int pid_received = read_cpu_interrupt_request(package);

        if(pid_received == *pid_on_execute) {

            t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
            buffer_add_uint32(buffer, pid_received);
            buffer_add_uint32(buffer, *pc_on_execute);

            t_package* package = package_create(CPU_INTERRUPT, buffer);
            send_package(socket_interrupt, package);
            package_destroy(package);
            mmu_process_cleanup(socket_memory);
            LOG_DEBUG("Interrupt for PID %d executed", pid_received);
                
            return 1;
        } else {
            LOG_DEBUG("Interrupt for PID %d received, but not executing", pid_received);
            // TODO: responder al kernel como que se "atendio" la interrupcion
        }

        send_cpu_interrupt_response(socket_interrupt, pid_received, *pc_on_execute);
    }
    else
    {
        LOG_WARNING("Received unexpected opcode on interrupt connection: %s", opcode_to_string(package->opcode) );
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
            LOG_INFO("Disconnecting interrupt listener...");
            return NULL;
        }
        lock_interrupt_list();
        LOG_DEBUG("Received interrupt package with opcode: %s", opcode_to_string(package->opcode));
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
            while(interrupt_count() > 0)
            {
                t_package* package = get_last_interrupt(interrupt_count());
                lock_cpu_mutex();
                check_interrupt(args->socket_interrupt, package, args->pid, args->pc args->socket_memory);
                unlock_cpu_mutex();
            }
        unlock_interrupt_list();
    }

    return NULL;
}
