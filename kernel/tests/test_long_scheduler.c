/*
#include "test_schedulers.h"
#include "kernel_logic/scheduler/long_scheduler.h"

// Contador para verificar las operaciones de SWAP-IN
int memory_swap_in_requests = 0;

void test_successful_long_scheduling_desuspension() {
    LOG_DEBUG("=== TEST: Desuspension Exitosa del Planificador de Largo Plazo ===");
    
    // Resetear contadores
    memory_swap_in_requests = 0;
    
    // Iniciar servidor mock
    start_memory_mock_server(&memory_swap_in_requests);
    
    // Crear un PCB y agregarlo a la lista SUSP_READY
    t_pcb* test_pcb_susp = create_test_pcb(200, 2048, "test_desuspension.txt");
    t_pcb* test_pcb_new = create_test_pcb(300, 2048, "test_desuspension.txt");
    add_pcb_to_susp_ready(test_pcb_susp);
    LOG_DEBUG("✓ PCB creado y agregado a SUSP_READY (PID: %u)\n", test_pcb_susp->pid);
    add_pcb_to_new(test_pcb_new);
    LOG_DEBUG("✓ PCB creado y agregado a NEW (PID: %u)\n", test_pcb_new->pid);    
    
    // Verificar estados iniciales
    assert(find_pcb_in_susp_ready(200));
    assert(!find_pcb_in_ready(200));
    assert(test_pcb_susp->current_state == SUSP_READY);
    
    assert(find_pcb_in_new(300));
    assert(!find_pcb_in_ready(300));
    assert(test_pcb_new->current_state == NEW);
    LOG_DEBUG("✓ Estado inicial verificado\n");

    // Ejecutar el planificador de largo plazo
    LOG_INFO("Ejecutando planificador de largo plazo");
    bool result = run_long_scheduler();
    
    // IMPORTANTE: Dar tiempo para que todas las conexiones se cierren
    usleep(200000); // 200ms
    
    // Verificaciones principales
    LOG_DEBUG("Verificando resultados");
    
    // Verificar que el planificador retornó true (indicando que inicializó procesos)
    assert(result == true);
    LOG_DEBUG("El planificador retorno true (procesos inicializados)\n");
    
    // Verificar que se hizo la solicitud de SWAP-IN a memoria
    assert(memory_swap_in_requests == 1);
    LOG_DEBUG("Se realizo 1 solicitud de SWAP-IN a memoria\n");
    
    // Verificar transición SUSPEND_READY -> READY
    assert(!find_pcb_in_susp_ready(200));
    LOG_DEBUG("El proceso ya no esta en la lista SUSP_READY\n");
    assert(find_pcb_in_ready(200));
    LOG_DEBUG("El proceso esta en la lista READY\n");
    assert(test_pcb_susp->current_state == READY);
    LOG_DEBUG("El estado del PCB es READY\n");
    
    // Verificar que el proceso NEW NO fue procesado (tiene menor prioridad)
    assert(find_pcb_in_new(300));
    assert(!find_pcb_in_ready(300));
    assert(test_pcb_new->current_state == NEW);
    LOG_DEBUG("Proceso PID 300: permanece en NEW (menor prioridad)\n");

    LOG_DEBUG("TEST EXITOSO: El proceso fue correctamente desuspendido y movido a READY\n");
    
    // Cleanup 
    pcb_destroyer(test_pcb_new);
    pcb_destroyer(test_pcb_susp);
}



int main() {
    printf("==========================================\n");
    printf(" TEST DEL PLANIFICADOR DE LARGO PLAZO\n");
    printf("     (DESUSPENSIÓN SUSP_READY -> READY)\n");
    printf("==========================================\n");
    
    // Inicializar configuración del kernel
    init_logger("long_scheduler_tester.log", "[LONG_SCHEDULER_TESTER]", LOG_LEVEL_DEBUG);
    LOG_DEBUG("Inicializando configuracion del kernel");
    
    t_config* config = init_config("kernel.config");
    kernel_config = init_kernel_config(config);
    
    // Inicializar las listas globales del kernel
    initialize_global_lists();
    initialize_global_semaphores();
    
    LOG_DEBUG("Configuracion completada. Iniciando tests");
    
    // Ejecutar el test
    test_successful_long_scheduling_desuspension();
    
    LOG_DEBUG("TODOS LOS TESTS COMPLETADOS EXITOSAMENTE");
    
    // Cleanup
    destroy_global_lists();
    destroy_global_repositories();
    config_destroy(config);
    destroy_logger();
    
    return 0;
}
*/