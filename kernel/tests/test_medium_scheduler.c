/*
#include "kernel_logic/scheduler/medium_scheduler.h"
#include "test_schedulers.h"

// Contador para verificar las operaciones
int memory_suspend_requests = 0;

void test_successful_medium_scheduling() {
    LOG_DEBUG("=== TEST: Planificacion de Mediano Plazo Exitosa ===");
    
    // Resetear contadores
    memory_suspend_requests = 0;
    
    // Iniciar servidor mock
    start_memory_mock_server(&memory_suspend_requests);
    
    // Crear un PCB y agregarlo a la lista BLOCKED
    t_pcb* test_pcb = create_test_pcb(100, 1024, "test_program.txt");
    add_pcb_to_blocked(test_pcb);
    
    printf("✓ PCB creado y agregado a BLOCKED (PID: %u)\n", test_pcb->pid);
    
    // Verificar que el PCB está en BLOCKED antes de ejecutar el scheduler
    assert(find_pcb_in_blocked(100));
    assert(!find_pcb_in_susp_blocked(100));
    
    // Ejecutar el planificador de mediano plazo
    LOG_INFO("Ejecutando planificador de mediano plazo");
    run_medium_scheduler(100);
    
    // IMPORTANTE: Dar tiempo para que todas las conexiones se cierren
    usleep(200000); // 200ms
    
    // Verificaciones principales
    LOG_DEBUG("Verificando resultados");
    
    // Verificar que se hizo la solicitud a memoria
    assert(memory_suspend_requests == 1);
    LOG_DEBUG("Se realizo 1 solicitud de suspension a memoria");
    
    // Verificar que el proceso no está más en BLOCKED
    assert(!find_pcb_in_blocked(100));
    LOG_DEBUG("El proceso ya no esta en la lista BLOCKED");
    
    // Verificar que el proceso está en SUSP_BLOCKED
    assert(find_pcb_in_susp_blocked(100));
    LOG_DEBUG("El proceso esta en la lista SUSP_BLOCKED");
    
    // Verificar el estado del PCB
    assert(test_pcb->current_state == SUSP_BLOCKED);
    LOG_DEBUG("El estado del PCB es SUSP_BLOCKED");
    
    LOG_DEBUG("TEST EXITOSO: El proceso fue correctamente suspendido");
    pcb_destroyer(test_pcb);
}

int main() {
    printf("========================================\n");
    printf(" TEST DEL PLANIFICADOR DE MEDIANO PLAZO\n");
    printf("========================================\n");
    
    // Inicializar configuración del kernel
    init_logger("tester.log", "[TESTER]", LOG_LEVEL_DEBUG);
    LOG_DEBUG("Inicializando configuracion del kernel");
    t_config* config = init_config("kernel.config");
    kernel_config = init_kernel_config(config);
    // Inicializar las listas globales del kernel
    initialize_global_lists();
    initialize_global_semaphores();
    LOG_DEBUG("Configuracion completada. Iniciando tests");
    
    // Ejecutar los tests
    test_successful_medium_scheduling();
    LOG_DEBUG("TODOS LOS TESTS COMPLETADOS EXITOSAMENTE");

    // Cleanup
    destroy_global_lists();
    destroy_global_repositories();
    config_destroy(config);
    destroy_logger();
    
    return 0;
}
*/