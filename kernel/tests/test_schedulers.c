#include "test_schedulers.h"

// Servidor mock que simula el módulo de memoria
void* memory_server_mock(void* arg) {
    int server_socket = create_server(kernel_config.memory_port);

    if (server_socket == -1) {
        printf("[MEMORY MOCK] Error creando servidor en puerto %s\n", kernel_config.memory_port);
        return NULL;
    }
    
    LOG_DEBUG("[MEMORY MOCK] Servidor iniciado en puerto %s\n", kernel_config.memory_port);
    
    // Esperar una sola conexión
    int client_socket = accept_connection("MEMORY MOCK", server_socket);
    if (client_socket < 0) {
        printf("[MEMORY MOCK] Error aceptando conexión\n");
        close(server_socket);
        return NULL;
    }
    
    LOG_DEBUG("[MEMORY MOCK] Cliente conectado\n");
    
    // Recibir el paquete de suspensión
    t_package* request = recv_package(client_socket);
    if (request != NULL) {
        int *cont = (int *) arg;
        (*cont)++;
        int32_t pid = read_swap_operation_package(request);
        printf("[MEMORY MOCK] Recibida solicitud de suspensión para PID %u\n", pid);
        
        // Enviar respuesta exitosa
        send_confirmation_package(client_socket, true);
        printf("[MEMORY MOCK] Respuesta enviada: ÉXITO\n");
        
        destroy_package(request);
    }
    
    // Esperar a que el cliente cierre la conexión (esto será cuando disconnect_from_memory() se ejecute)
    char buffer[1];
    int bytes_received = recv(client_socket, buffer, 1, 0);
    if (bytes_received == 0) {
        printf("[MEMORY MOCK] Cliente cerró la conexión\n");
    }
    
    close(client_socket);
    close(server_socket);
    printf("[MEMORY MOCK] Servidor cerrado\n");
    
    return NULL;
}

void start_memory_mock_server(int* arg) { 
    pthread_t memory_server_thread;
    if (pthread_create(&memory_server_thread, NULL, memory_server_mock, arg) != 0) {
        LOG_DEBUG("[MEMORY MOCK] Error creando hilo del servidor");
        return;
    }
    
    // Detach el hilo para que se limpie automáticamente cuando termine
    pthread_detach(memory_server_thread);
    
    // Esperar que el servidor esté listo
    usleep(100000); // 100ms
    LOG_DEBUG("[MEMORY MOCK] Servidor mock iniciado correctamente");
}

t_pcb* create_test_pcb(int32_t pid, int32_t size, const char* filename){
    t_pcb* pcb = pcb_create(pid, 0, size, (char *) filename);
    return pcb;
}