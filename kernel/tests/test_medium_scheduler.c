#include "../utils.h"
#include "../src/handlers/scheduler/medium_scheduler.h"
#include <assert.h>

// Variable global del kernel
t_kernel_config kernel_config;

// Contador para verificar las operaciones
static int memory_suspend_requests = 0;

// Servidor mock que simula el módulo de memoria
void* memory_server_mock(void* arg) {
    int server_port = kernel_config.memory_port;
    int server_socket = create_server(server_port);
    
    if (server_socket == -1) {
        printf("[MEMORY MOCK] Error creando servidor en puerto %d\n", server_port);
        return NULL;
    }
    
    printf("[MEMORY MOCK] Servidor iniciado en puerto %d\n", server_port);
    
    // Esperar una sola conexión
    int client_socket = accept_connection(server_socket);
    if (client_socket < 0) {
        printf("[MEMORY MOCK] Error aceptando conexión\n");
        close(server_socket);
        return NULL;
    }
    
    printf("[MEMORY MOCK] Cliente conectado\n");
    
    // Recibir el paquete de suspensión
    t_package* request = recv_package(client_socket);
    if (request != NULL) {
        memory_suspend_requests++;
        uint32_t pid = read_memory_suspend_process(request);
        printf("[MEMORY MOCK] Recibida solicitud de suspensión para PID %u\n", pid);
        
        // Enviar respuesta exitosa
        send_memory_suspend_process_response(client_socket, 1);
        printf("[MEMORY MOCK] Respuesta enviada: ÉXITO\n");
        
        package_destroy(request);
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

void start_memory_mock_server() {
    memory_suspend_requests = 0;
    
    pthread_t memory_server_thread;
    if (pthread_create(&memory_server_thread, NULL, memory_server_mock, NULL) != 0) {
        LOG_DEBUG("[MEMORY MOCK] Error creando hilo del servidor");
        return;
    }
    
    // Detach el hilo para que se limpie automáticamente cuando termine
    pthread_detach(memory_server_thread);
    
    // Esperar que el servidor esté listo
    usleep(100000); // 100ms
    LOG_DEBUG("[MEMORY MOCK] Servidor mock iniciado correctamente");
}

t_pcb* create_test_pcb(uint32_t pid, uint32_t size, const char* filename) {
    char* file_copy = malloc(strlen(filename) + 1);
    strcpy(file_copy, filename);
    
    t_pcb* pcb = pcb_create(pid, 0, size, file_copy);
    pcb_change_state(pcb, BLOCKED);
    return pcb;
}

void test_successful_medium_scheduling() {
    LOG_INFO("=== TEST: Planificación de Mediano Plazo Exitosa ===");
    
    // Resetear contadores
    memory_suspend_requests = 0;
    
    // Iniciar servidor mock
    start_memory_mock_server();
    
    // Crear un PCB y agregarlo a la lista BLOCKED
    t_pcb* test_pcb = create_test_pcb(100, 1024, "test_program.txt");
    add_pcb_to_blocked(test_pcb);
    
    printf("✓ PCB creado y agregado a BLOCKED (PID: %u)\n", test_pcb->pid);
    
    // Verificar que el PCB está en BLOCKED antes de ejecutar el scheduler
    assert(find_pcb_in_blocked(100));
    assert(!find_pcb_in_susp_blocked(100));
    
    // Ejecutar el planificador de mediano plazo
    printf("Ejecutando planificador de mediano plazo...\n");
    run_medium_scheduler(100, 1);
    
    // IMPORTANTE: Dar tiempo para que todas las conexiones se cierren
    usleep(200000); // 200ms
    
    // Verificaciones principales
    printf("Verificando resultados...\n");
    
    // Verificar que se hizo la solicitud a memoria
    assert(memory_suspend_requests == 1);
    printf("✓ Se realizó 1 solicitud de suspensión a memoria\n");
    
    // Verificar que el proceso no está más en BLOCKED
    assert(!find_pcb_in_blocked(100));
    printf("✓ El proceso ya no está en la lista BLOCKED\n");
    
    // Verificar que el proceso está en SUSP_BLOCKED
    assert(find_pcb_in_susp_blocked(100));
    printf("✓ El proceso está en la lista SUSP_BLOCKED\n");
    
    // Verificar el estado del PCB
    assert(find_pcb_in_susp_blocked(100));
    assert(test_pcb->current_state == SUSP_BLOCKED);
    printf("✓ El estado del PCB es SUSP_BLOCKED\n");
    
    printf("✓ TEST EXITOSO: El proceso fue correctamente suspendido\n");
    pcb_destroyer(test_pcb);
}

int main() {
    printf("========================================\n");
    printf("  TEST DEL PLANIFICADOR DE MEDIANO PLAZO\n");
    printf("========================================\n");
    
    // Inicializar configuración del kernel
    printf("Inicializando configuración del kernel...\n");
    t_config* config = init_config("kernel.config");
    kernel_config = init_kernel_config(config);
    init_logger("tester.log", "TESTER", LOG_LEVEL_INFO);

    // Inicializar las listas globales del kernel
    initialize_global_lists();
    initialize_global_semaphores();
    LOG_INFO("Configuración completada. Iniciando tests...\n");
    
    // Ejecutar los tests
    test_successful_medium_scheduling();
    printf("\n");
    LOG_INFO("TODOS LOS TESTS COMPLETADOS EXITOSAMENTE");

    // Cleanup
    destroy_global_lists();
    destroy_global_repositories();
    config_destroy(config);
    destroy_logger();
    
    return 0;
}