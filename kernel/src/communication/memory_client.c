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

bool create_process_in_memory(int memory_socket, uint32_t pid, uint32_t size, char* pseudocode_path) {
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
    int sent_bytes = send_memory_create_process_request(memory_socket, pid, size, pseudocode_path);
    
    if (sent_bytes <= 0) {
        LOG_ERROR("memory_client: Error enviando paquete CREATE_PROCESS");
        return false;
    }
    
    LOG_INFO("memory_client: Paquete CREATE_PROCESS enviado (%d bytes)", sent_bytes);
    
    // Recibir respuesta de memoria
    t_package* response = recv_package(memory_socket);
    if (response == NULL) {
        LOG_ERROR("memory_client: Error recibiendo respuesta de memoria");
        return false;
    }
    bool success = false;
    if (response->opcode == CREATE_PROCESS) {
        bool create_process_result = read_memory_create_process_response(response);
        success = create_process_result;
    } else {
        LOG_ERROR("memory_client: Respuesta inesperada de memoria (opcode: %d)", response->opcode);
    }
    
    package_destroy(response);
    return success;
}

void disconnect_from_memory(int memory_socket) {
    if (memory_socket >= 0) {
        LOG_INFO("memory_client: Cerrando conexión con memoria (socket %d)", memory_socket);
        close(memory_socket);
    }
}
