#include "instructionCycle.h"

t_package* fetch(int socket, uint32_t PID, uint32_t PC) 
{
    log_debug(get_logger(), "Fetching instruction for PID: %d, PC: %d", PID, PC);
    t_package* package;
    do
    {
        request_instruction(socket, PID, PC);
        package = receive_instruction(socket);

    }while(package != NULL);

    return package;
}

t_instruction* decode(t_package* package) 
{
    t_instruction* instruction = safe_malloc(sizeof(t_instruction));

    package->buffer->offset = 0;
    instruction->instruction_code = buffer_read_uint32(package->buffer);
    
    switch(instruction->instruction_code)
    {
        case NOOP:
            break;
        case WRITE:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            instruction->operand_string_size = buffer_read_uint32(package->buffer);
            instruction->operand_string = buffer_read_string(package->buffer, &instruction->operand_string_size);
        case READ:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            instruction->operand_numeric2 = buffer_read_uint32(package->buffer);
            break;
        case GOTO:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            break;
        default:
            break;
    }

    return instruction;
}

int execute(t_instruction* instruction, t_package* instruction_package, int socket_memory, int socket_dispatch, uint32_t* PC) 
{
    switch (instruction->instruction_code) {
        case NOOP:
            // No operation
            break;
        case WRITE:
                uint32_t logic_dir_write = instruction->operand_numeric1;
                char* valor_write = instruction->operand_string;
                uint32_t physic_dir_write = MMU(logic_dir_write);
                write_memory_request(socket_memory, physic_dir_write, valor_write);
            break;
        case READ:
                uint32_t logic_dir_read = instruction->operand_numeric1;
                uint32_t size = instruction->operand_numeric2;
                uint32_t physic_dir_read = MMU(logic_dir_read);
                read_memory_request(socket_memory, physic_dir_read, size);
                char* data = read_memory_response(socket_memory);
                if(data != NULL)
                {
                    log_info(get_logger(), "Data read from memory: %s", data);
                    free(data);
                }else{
                    log_error(get_logger(), "Failed to read data from memory");
                    return -1;
                }
            break;
        case GOTO:
                *PC = instruction->operand_numeric1;
            break;
        case IO:
            // TODO: esto esta mal, deberiamos mandar una nueva instruccion. sino se va a mandar el opcode GET_INSTRUCTION que es el que se le mandó a la memoria desde la CPU y con el que la memoria contestó
            send_package(socket_dispatch, instruction_package);
            break;
        case INIT_PROC:
                send_package(socket_dispatch, instruction_package);
            break;
        case DUMP_PROCESS:
                send_package(socket_dispatch, instruction_package);
            break;
        case EXIT:
                send_package(socket_dispatch, instruction_package);
            break;
        default:
                log_error(get_logger(), "Unknown instruction: %d", instruction->instruction_code);
            return -1;
    }

    PC++;

    return 0;
}

int check_interrupt(int socket_interrupt, int pid_on_execute, int pc_on_execute) 
{
    extern sem_t cpu_mutex; // en main

    t_package* package = recv_package(socket_interrupt);
    if(package == NULL) 
    {
        log_error(get_logger(), "Disconnecting interrupt...");
        return -1;
    }

    if(package->opcode == CPU_INTERRUPT) 
    {
        sem_wait(&cpu_mutex);

        log_info(get_logger(), "Received interrupt from kernel");
        package->buffer->offset = 0;
        int pid_received = buffer_read_uint32(package->buffer);

        if(pid_received == pid_on_execute) {
            log_info(get_logger(), "Interrupt for PID %d received", pid_received);

            t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
            buffer_add_uint32(buffer, pid_received);
            buffer_add_uint32(buffer, pc_on_execute);

            t_package* package = package_create(CPU_INTERRUPT, buffer);
            send_package(socket_interrupt, package);
            package_destroy(package);

            log_info(get_logger(), "Interrupt for PID %d executed", pid_received);
            return 1;
        } else {
            log_info(get_logger(), "Interrupt for PID %d received, but not executing", pid_received);
            // TODO: responder al kernel como que se "atendio" la interrupcion
        }

        sem_post(&cpu_mutex);
        
    } 
    else 
    {
        log_error(get_logger(), "Received unexpected package: %d. Expecting Interrupt.", package->opcode);
    }
    
    return 0;
}

void* interrupt_handler(void* thread_args) 
{   
    interrupt_args_t* args = (interrupt_args_t *) thread_args;

    while( check_interrupt(args->socket_interrupt , *args->pid, *args->pc) != -1);
    

    return NULL;
}

uint32_t MMU(uint32_t logic_dir) 
{
    t_config* config_memoria = init_config("../memoria/memoria.config");
    uint32_t levels = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    uint32_t entrys_by_table = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    uint32_t size_pag = config_get_int_value(config_memoria, "TAM_PAGINA");

    uint32_t num_page = logic_dir / size_pag;
    uint32_t offset = logic_dir % size_pag;
    uint32_t physic_dir = 0;
    
    for(int actual_level = 1; actual_level <= levels; actual_level++) 
    {
        uint32_t divisor = pow(entrys_by_table, levels - actual_level);
        uint32_t entry = (num_page / divisor) % entrys_by_table;
        physic_dir = physic_dir * entrys_by_table + entry;
    }
    
    return (uint32_t)(physic_dir * size_pag + offset);
}