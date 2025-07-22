#include "memory_client.h"

int connect_to_memory(t_kernel_config* config) {
    if (config == NULL) {
        LOG_ERROR("memory_client: config es NULL");
        return -1;
    }
    
    LOG_INFO("memory_client: Conectando a memoria en %s:%s", 
             config->memory_ip, config->memory_port);
    
    int memory_socket = create_connection(config->memory_port, config->memory_ip);
    if (memory_socket == -1) {
        LOG_ERROR("memory_client: Error conectando a memoria");
        return -1;
    }
    
    LOG_INFO("memory_client: Conexión exitosa con memoria (socket %d)", memory_socket);
    return memory_socket;
}

bool create_process(int memory_socket, int32_t pid, int32_t size, char* pseudocode_path) {
    if (memory_socket < 0) {
        LOG_ERROR("memory_client: Socket inválido");
        return false;
    }
    
    if (pseudocode_path == NULL) {
        LOG_ERROR("memory_client: Ruta de pseudocódigo es NULL");
        return false;
    }
    
    LOG_INFO("memory_client: Creando proceso PID=%d, size=%d, path=%s", 
             pid, size, pseudocode_path);
    
    // Enviar paquete usando DTO
    int sent_bytes = send_init_process_package(memory_socket, pid, size, pseudocode_path);
    
    if (sent_bytes <= 0) {
        LOG_ERROR("memory_client: Error enviando paquete CREATE_PROCESS");
        return false;
    }
    
    // Recibir respuesta de memoria
    t_package* response = recv_package(memory_socket);
    if (response == NULL) {
        LOG_ERROR("memory_client: Error recibiendo respuesta de memoria");
        return false;
    }
    bool success = false;
    if (response->opcode == CONFIRMATION) {
        success = read_confirmation_package(response);
    } else {
        LOG_ERROR("memory_client: Respuesta inesperada de memoria (opcode: %d)", response->opcode);
    }
    
    destroy_package(response);
    return success;
}

int kill_process_in_memory(int32_t pid)
{
    extern t_kernel_config kernel_config; // en globals.h

    int memsocket = connect_to_memory(&kernel_config);
    send_kill_process_package(memsocket, pid);

    t_package* response = recv_package(memsocket);
    disconnect_from_memory(memsocket);

    if (response->opcode == CONFIRMATION)
    {
        destroy_package(response);
        return true;
    }
    else
        return false;
}

bool dump_memory_routine(int32_t pid)
{
    extern t_kernel_config kernel_config; // en globals.h

    int memsocket = connect_to_memory(&kernel_config);
    if (memsocket < 0) return false;
    
    send_dump_memory_package(memsocket, pid);

    t_package* response = recv_package(memsocket);
    disconnect_from_memory(memsocket);

    if (response == NULL) return false;

    if (response->opcode == CONFIRMATION)
    {
        bool status = read_confirmation_package(response);
        destroy_package(response);
        return status;
    }

    destroy_package(response);
    return false;

}

void disconnect_from_memory(int memory_socket) {
    if (memory_socket >= 0) {
        LOG_INFO("memory_client: Cerrando conexión con memoria (socket %d)", memory_socket);
        close(memory_socket);
    }
}
