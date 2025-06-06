
#include <stdlib.h>
#include <semaphore.h>
#include "../utils.h"

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

    int PID = 6,PC = 20;

    // =========================
    // Creación de servidores
    // =========================

    int memory_server = create_server("80011");
    int dispatch_server = create_server("30003");
    int interrupt_server = create_server("30004");

    LOG_INFO("Servers created successfully!");

    // =========================
    // Esperando conexiones de CPU
    // =========================

    int cpu_memory_connection = dummy_connection(memory_server);
    int cpu_dispatch_connection = dummy_connection(dispatch_server);
    int cpu_interrupt_connection = dummy_connection(interrupt_server);

    LOG_INFO("Connections established successfully!");

    // =========================
    // Enviando IO COMPLETION
    // =========================

    printf("\n\x1b[33mPress Enter to send IO COMPLETION to CPU dispatch connection.\x1b[0m\n");
    getchar();

    send_io_operation_completed(cpu_dispatch_connection, "1");
    LOG_INFO("IO COMPLETION sent to CPU dispatch connection.");
    
    // =========================
    // Enviando PID & PC
    // =========================

    printf("\n\x1b[33mPress Enter to send PID & PC to CPU dispatch connection.\x1b[0m\n");
    getchar();

    send_cpu_dispatch_request(cpu_dispatch_connection, PID, PC);
    LOG_INFO("Package sent with opcode PID_PC_PACKAGE to CPU dispatch connection.");

    // =========================
    // Esperando respuesta de CPU memory
    // =========================

    LOG_INFO("Waiting for response from CPU memory connection...");
    response = recv_package(cpu_memory_connection);
    LOG_INFO("Received package from CPU memory connection with opcode: %s", opcode_to_string(response->opcode));
    package_destroy(response);

    // =========================
    // Enviando instrucción
    // =========================

    printf("\n\x1b[33mPress Enter to send NOOP to CPU memory connection.\x1b[0m\n");
    getchar();

    send_memory_instruction_package(cpu_memory_connection, "NOOP");
    LOG_INFO("NOOP sent to CPU memory connection.");

    // =========================
    // Enviando syscall  EXIT
    // =========================

    printf("\n\x1b[33mPress Enter to send instruction to CPU memory connection.\x1b[0m\n");
    getchar();

    send_memory_instruction_package(cpu_memory_connection, "EXIT");
    LOG_INFO("Syscall sent to CPU memory connection.");

    // ===================================
    // Recbiendo respuesta de CPU dispatch
    // ===================================

    response = recv_package(cpu_dispatch_connection);
    LOG_INFO("Received package from CPU dispatch connection with opcode: %s", opcode_to_string(response->opcode));
    package_destroy(response);


    // ===================================
    // Mandando interrupts
    // ===================================

    printf("\n\x1b[33mPress Enter to send same PID interruption\x1b[0m\n");
    getchar();

    send_cpu_interrupt_request(cpu_interrupt_connection, PID);

    printf("\n\x1b[33mPress Enter to send other PID interruption\x1b[0m\n");
    getchar();

    send_cpu_interrupt_request(cpu_interrupt_connection, PID + 1);

    printf("\n\x1b[33mPress Enter to send PID/PC and interruption simultaneously\x1b[0m\n");
    getchar();

    LOG_INFO("Sending PID/PC");
    send_cpu_dispatch_request(cpu_dispatch_connection, PID, PC);
    LOG_INFO("Sending Interruption");
    send_cpu_interrupt_request(cpu_interrupt_connection, PID);

    LOG_INFO("Sending Instruction NOOP to CPU memory connection and interruption to CPU interrupt connection");
    send_memory_instruction_package(cpu_memory_connection, "NOOP");
    // send_cpu_interrupt_request(cpu_interrupt_connection, PID);
    
    response = recv_package(cpu_memory_connection);
    LOG_INFO("Received package from CPU memory connection with opcode: %s", opcode_to_string(response->opcode));
    package_destroy(response);

    // =========================
    // Finalizando tester
    // =========================

    printf("\n\x1b[33mPress Enter to exit...\x1b[0m\n");
    getchar();

    return 0;
}