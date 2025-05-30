#include "instructionCycle.h"

t_package* fetch(int socket, uint32_t PID, uint32_t PC) 
{
    t_package* package;
    do
    {
        request_instruction(socket, PID, PC);
        package = receive_instruction(socket);

    }while(package != NULL);

    return package;
}

instruction_t* decode(t_package* package) 
{
    instruction_t* instruction = safe_malloc(sizeof(instruction_t));

    package->buffer->offset = 0;
    // TODO: la memoria manda asi nomas toda la instruccion (por ej: "IO teclado 1000"). aca la cpu debe interpretarla
    // y hacer lo correspondiente dependiendo de que instruccion se trata.
    // osea, tenemos que implementar la logica correspondiente para interpretar cualquier instruccion.
    instruction->cod_instruction = buffer_read_uint32(package->buffer);
    
    switch(instruction->cod_instruction)
    {
        case INST_NOOP:
            break;
        case INST_WRITE:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            instruction->operand_string_size = buffer_read_uint32(package->buffer);
            instruction->operand_string = buffer_read_string(package->buffer, &instruction->operand_string_size);
            break;
        case INST_READ:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            instruction->operand_numeric2 = buffer_read_uint32(package->buffer);
            break;
        case INST_GOTO:
            instruction->operand_numeric1 = buffer_read_uint32(package->buffer);
            break;
        default:
            break;
    }

    return instruction;
}

int execute(instruction_t* instruction, t_package* instruction_package, int socket_memory, int socket_dispatch, uint32_t pid, uint32_t* PC) 
{
    switch (instruction->cod_instruction) {
        case INST_NOOP:
            // No operation
            break;
        case INST_WRITE:
                uint32_t logic_dir_write = instruction->operand_numeric1;
                char* valor_write = instruction->operand_string;
                uint32_t physic_dir_write = MMU(logic_dir_write);
                write_memory_request(socket_memory, physic_dir_write, valor_write);
            break;
        case INST_READ:
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
        case INST_GOTO:
                *PC = instruction->operand_numeric1;
            break;
        case INST_IO:
            // Enviar syscall IO usando nuevo DTO
            send_cpu_syscall_request(socket_dispatch, SYSCALL_RESPONSE_IO, pid, *PC);
            // Para IO, CPU se autodesaloja (no espera respuesta)
            return 1; // Indica que debe retornar el control al kernel
        case INST_INIT_PROC:
            // Enviar syscall INIT_PROC usando nuevo DTO
            send_cpu_syscall_request(socket_dispatch, SYSCALL_RESPONSE_INIT_PROC, pid, *PC);
            // Para INIT_PROC, CPU debe esperar respuesta del kernel
            // Aquí esperaríamos una respuesta específica del kernel
            // TODO: esperar respuesta del kernel
            break;
        case INST_DUMP_PROCESS:
            // Enviar syscall DUMP_PROCESS usando nuevo DTO
            send_cpu_syscall_request(socket_dispatch, SYSCALL_RESPONSE_DUMP_PROCESS, pid, *PC);
            // Para DUMP_PROCESS, CPU se autodesaloja (no espera respuesta)
            return 1; // Indica que debe retornar el control al kernel
        case INST_EXIT:
            // Enviar syscall EXIT usando nuevo DTO
            send_cpu_syscall_request(socket_dispatch, SYSCALL_RESPONSE_EXIT, pid, *PC);
            // Para EXIT, CPU se autodesaloja (no espera respuesta)
            return 1; // Indica que debe retornar el control al kernel
        default:
                log_error(get_logger(), "Unknown instruction: %d", instruction->cod_instruction);
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
        } else {
            log_info(get_logger(), "Interrupt for PID %d received, but not executing", pid_received);
        }
        
        send_cpu_interrupt_response(socket_interrupt, pid_received, pc_on_execute);

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