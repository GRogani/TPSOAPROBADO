#include "instructionCycle.h"
#include "mmu.h"
#include <pthread.h>

extern CacheConfig *g_cache_config;
extern TLBConfig *g_tlb_config;
extern MMUConfig *g_mmu_config;

t_package *fetch(int socket, uint32_t PID, uint32_t PC)
{
    t_package *package;

    LOG_OBLIGATORIO("## PID: %d - FETCH - Program Counter: %d", PID, PC);

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
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: NOOP -", *pid);
    {
        // No operation
        (*PC)++;
        break;
    }
    case WRITE:
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: WRITE - %d %s", *pid, instruction->operand_numeric1, instruction->operand_string);
    {
        uint32_t logic_dir_write = instruction->operand_numeric1;
        char *valor_write = instruction->operand_string;
        uint32_t page_number = floor(logic_dir_write / g_mmu_config->page_size);
        uint32_t offset = logic_dir_write % g_mmu_config->page_size;
        if (g_cache_config->entry_count > 0)
        {
            // If cache is enabled, we need to write to cache first
            CacheEntry *cache_entry = cache_find_entry(page_number, *pid);
            if (cache_entry == NULL)
            {
                cache_entry = select_victim_entry(socket_memory, logic_dir_write, *pid);
                cache_entry->is_valid = true;
                cache_entry->page = page_number;
                cache_entry->use_bit = true;
            }
            // Write to cache
            memcpy(cache_entry->content, valor_write, instruction->operand_string_size);
            cache_entry->modified_bit = true;
            (*PC)++;
            break;
        }
        else
        {
            if (g_tlb_config->entry_count > 0)
            {
                uint32_t physic_dir_write = mmu_translate_address(socket_memory, logic_dir_write, *pid);
                t_memory_write_request *write_req = create_memory_write_request(physic_dir_write, instruction->operand_string_size, valor_write);
                send_memory_write_request(socket_memory, write_req);
                destroy_memory_write_request(write_req);
                (*PC)++;
                break;
            }
            else
            {
                uint32_t frame_number = mmu_perform_page_walk(socket_memory, page_number, *pid);
                uint32_t physic_dir_write = (frame_number * g_mmu_config->page_size) + offset;
                t_memory_write_request *write_req = create_memory_write_request(physic_dir_write, instruction->operand_string_size, valor_write);
                send_memory_write_request(socket_memory, write_req);
                destroy_memory_write_request(write_req);
                (*PC)++;
                break;
            }
        }
    }
    case READ:
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: READ - %d %d", *pid, instruction->operand_numeric1, instruction->operand_numeric2);
    {
        uint32_t logic_dir_read = instruction->operand_numeric1;
        uint32_t size = instruction->operand_numeric2;
        uint32_t page_number = floor(logic_dir_read / g_mmu_config->page_size);
        uint32_t offset = logic_dir_read % g_mmu_config->page_size;
        if (g_cache_config->entry_count > 0)
        {
            // If cache is enabled, we need to read from cache first
            CacheEntry *cache_entry = cache_find_entry(page_number, *pid);
            if (cache_entry == NULL)
            {
                
                cache_entry = select_victim_entry(socket_memory, logic_dir_read, *pid);
                cache_entry = cache_load_page(logic_dir_read, socket_memory, cache_entry, *pid);
            }
            // Read from cache
            LOG_INFO("Data read from cache: %p", cache_entry->content);
            (*PC)++;
            break;
        }
        else
        {
            LOG_INFO("Cache is disabled, reading directly from memory");
            if (g_tlb_config->entry_count > 0)
            {
                uint32_t physic_dir_read = mmu_translate_address(socket_memory, logic_dir_read, *pid);
                t_memory_read_request *request = create_memory_read_request(physic_dir_read, size);
                send_memory_read_request(socket_memory, request);
                destroy_memory_read_request(request);

                t_package *package = recv_package(socket_memory);
                if (package == NULL || package->opcode != READ_MEMORY)
                {
                    LOG_INFO("Failed to read data from memory");
                    if (package)
                        destroy_package(package);
                    return -1;
                }

                t_memory_read_response *response = read_memory_read_response(package);
                destroy_package(package);

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
            else
            {
                uint32_t frame_number = mmu_perform_page_walk(socket_memory, page_number, *pid);
                uint32_t physic_dir_read = (frame_number * g_mmu_config->page_size) + offset;
                t_memory_read_request *request = create_memory_read_request(physic_dir_read, size);
                send_memory_read_request(socket_memory, request);
                destroy_memory_read_request(request);

                t_package *package = recv_package(socket_memory);
                if (package == NULL || package->opcode != READ_MEMORY)
                {
                    LOG_INFO("Failed to read data from memory");
                    if (package)
                        destroy_package(package);
                    return -1;
                }

                t_memory_read_response *response = read_memory_read_response(package);
                destroy_package(package);

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
        }
    }
    case GOTO:
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: GOTO - %d", *pid, instruction->operand_numeric1);
    {
        *PC = instruction->operand_numeric1;
        break;
    }
    case IO:
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: IO - %s %d", *pid, instruction->operand_string, instruction->operand_numeric1);
    {

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
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: INIT_PROC - %s %d", *pid, instruction->operand_string, instruction->operand_numeric1);
    {
        (*PC)++;
        syscall_package_data *syscall_req = safe_malloc(sizeof(syscall_package_data));
        syscall_req->syscall_type = SYSCALL_INIT_PROC;
        syscall_req->pid = *pid;
        syscall_req->pc = *PC;
        syscall_req->params.init_proc.pseudocode_file = strdup(instruction->operand_string);
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
        bool success = read_confirmation_package(package);
        if (!success)
        {
            LOG_ERROR("Failed to initialize process for PID %d", *pid);
            destroy_package(package);
            return true;
        }
        break;
    }
    case DUMP_PROCESS:
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: DUMP_PROCESS -", *pid);
    {
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
        LOG_OBLIGATORIO("## PID: %d - Ejecutando: EXIT -", *pid);
    {
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

void check_interrupt(int socket_interrupt, t_package *package, uint32_t *pid_on_execute, uint32_t *pc_on_execute, int socket_memory)
{
    if (package->opcode == INTERRUPT)
    {
        LOG_OBLIGATORIO("## Llega interrupción al puerto Interrupt");
        int pid_received = read_interrupt_package(package);

        if (pid_received == *pid_on_execute)
        {

            send_cpu_context_package(socket_interrupt, pid_received, *pc_on_execute, 0);

            LOG_INFO("Interrupt for PID %d executed", pid_received);

            mmu_process_cleanup(socket_memory);

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
    interrupt_args_t *thread_args = (interrupt_args_t *)args;

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
        check_interrupt(args->socket_interrupt, package, args->pid, args->pc, args->socket_memory);
        destroy_package(package);
    }
    unlock_interrupt_list();

    LOG_INFO("Interrupt found: %s", interrupted ? "yes" : "no");

    return interrupted;
}