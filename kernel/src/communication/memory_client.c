#include "memory_client.h"

int connect_to_memory(t_kernel_config* config) {
    if (config == NULL) {
        log_error(get_logger(), "memory_client: config es NULL");
        return -1;
    }
    
    log_info(get_logger(), "memory_client: Conectando a memoria en %s:%s", 
             config->memory_ip, config->memory_port);
    
    int memory_socket = create_connection(config->memory_port, config->memory_ip);
    if (memory_socket == -1) {
        log_error(get_logger(), "memory_client: Error conectando a memoria");
        return -1;
    }
    
    log_info(get_logger(), "memory_client: Conexión exitosa con memoria (socket %d)", memory_socket);
    return memory_socket;
}

bool create_process_in_memory(int memory_socket, uint32_t pid, uint32_t size, const char* pseudocode_path) {
    if (memory_socket < 0) {
        log_error(get_logger(), "memory_client: Socket inválido");
        return false;
    }
    
    if (pseudocode_path == NULL) {
        log_error(get_logger(), "memory_client: Ruta de pseudocódigo es NULL");
        return false;
    }
    
    log_info(get_logger(), "memory_client: Creando proceso PID=%d, size=%d, path=%s", 
             pid, size, pseudocode_path);
    
    // Enviar paquete usando DTO
    int sent_bytes = send_memory_create_process_request(memory_socket, pid, size, pseudocode_path);
    
    if (sent_bytes <= 0) {
        log_error(get_logger(), "memory_client: Error enviando paquete CREATE_PROCESS");
        return false;
    }
    
    log_info(get_logger(), "memory_client: Paquete CREATE_PROCESS enviado (%d bytes)", sent_bytes);
    
    // Recibir respuesta de memoria
    t_package* response = recv_package(memory_socket);
    if (response == NULL) {
        log_error(get_logger(), "memory_client: Error recibiendo respuesta de memoria");
        return false;
    }
    
    bool success = false;
    if (response->opcode == CREATE_PROCESS) {
        // Leer resultado de la operación
        // TODO: ver si deberiamos moverlo al dto (la interpretacion del resultado)
        if (response->buffer != NULL && response->buffer->stream_size >= sizeof(uint32_t)) {
            response->buffer->offset = 0;
            uint32_t result = buffer_read_uint32(response->buffer);
            success = (result == 0); // 0 = éxito, otro valor = error
            
            if (success) {
                log_info(get_logger(), "memory_client: Proceso PID=%d creado exitosamente en memoria", pid);
            } else {
                log_error(get_logger(), "memory_client: Error creando proceso PID=%d en memoria (código: %d)", 
                         pid, result);
            }
        } else {
            log_warning(get_logger(), "memory_client: Respuesta de memoria sin datos de resultado");
            // Por compatibilidad, asumir éxito si no hay código de error explícito
            success = true;
        }
    } else {
        log_error(get_logger(), "memory_client: Respuesta inesperada de memoria (opcode: %d)", response->opcode);
    }
    
    package_destroy(response);
    return success;
}

void disconnect_from_memory(int memory_socket) {
    if (memory_socket >= 0) {
        log_info(get_logger(), "memory_client: Cerrando conexión con memoria (socket %d)", memory_socket);
        close(memory_socket);
    }
}
