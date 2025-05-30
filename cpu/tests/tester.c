
#include <semaphore.h>
#include "../utils.h"


sem_t cpu_mutex;

int dummy_connection(int server_socket) 
{
    int connection;

    do
    {
        connection = accept_connection(server_socket);
        if (connection < 0)
            printf("Waiting for connection...\n");

    }while(connection < 0);

    return connection;
}

int main(){
    // =========================
    // Inicialización del logger
    // =========================
    init_logger("tester.log", "TESTER", LOG_LEVEL_INFO);

    t_package* response;
    t_instruction* instruction = create_instruction(NOOP, 0, 0, 0, NULL);
    t_instruction* syscall = create_instruction(EXIT, 0, 0, 0, NULL);

    // =========================
    // Creación de servidores
    // =========================
    int memory_server = create_server("30002");
    int dispatch_server = create_server("30003");
    int interrupt_server = create_server("30004");

    log_info(get_logger(), "Servers created successfully!");

    // =========================
    // Esperando conexiones de CPU
    // =========================
    int cpu_memory_connection = dummy_connection(memory_server);
    int cpu_dispatch_connection = dummy_connection(dispatch_server);
    int cpu_interrupt_connection = dummy_connection(interrupt_server);

    log_info(get_logger(), "Connections established successfully!");

    // =========================
    // Enviando IO COMPLETION
    // =========================
    printf("\n\x1b[33mPress Enter to send IO COMPLETION to CPU dispatch connection.\x1b[0m\n");
    getchar();

    send_io_operation_completed(cpu_dispatch_connection, "1");
    log_info(get_logger(), "IO COMPLETION sent to CPU dispatch connection.");
    
    // =========================
    // Enviando PID & PC
    // =========================
    printf("\n\x1b[33mPress Enter to send PID & PC to CPU dispatch connection.\x1b[0m\n");
    getchar();

    send_PID_PC(cpu_dispatch_connection, 1, 3);
    log_info(get_logger(), "Package sent with opcode PID_PC_PACKAGE to CPU dispatch connection.");

    // =========================
    // Esperando respuesta de CPU memory
    // =========================
    log_debug(get_logger(), "Waiting for response from CPU memory connection...");
    response = recv_package(cpu_memory_connection);
    log_info(get_logger(), "Received package from CPU memory connection with opcode: %s", opcode_to_string(response->opcode));
    package_destroy(response);

    // =========================
    // Enviando instrucción
    // =========================
    printf("\n\x1b[33mPress Enter to send NOOP to CPU memory connection.\x1b[0m\n");
    getchar();

    send_instruction(cpu_memory_connection, instruction);
    log_info(get_logger(), "Instruction sent to CPU memory connection.");

    // =========================
    // Enviando syscall  EXIT
    // =========================
    printf("\n\x1b[33mPress Enter to send instruction to CPU memory connection.\x1b[0m\n");
    getchar();

    send_instruction(cpu_memory_connection, syscall);
    log_info(get_logger(), "Syscall sent to CPU memory connection.");

    // ===================================
    // Recbiendo respuesta de CPU dispatch
    // ===================================
    response = recv_package(cpu_dispatch_connection);
    log_info(get_logger(), "Received package from CPU dispatch connection with opcode: %s", opcode_to_string(response->opcode));
    package_destroy(response);
    // =========================
    // Finalizando tester
    // =========================
    printf("\n\x1b[33mPress Enter to exit...\x1b[0m\n");
    getchar();

    return 0;
}