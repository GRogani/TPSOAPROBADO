#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "../src/swap_space/swap_manager.h"
#include "../../utils/utils.h"

// Configuración para las pruebas
t_memoria_config test_config = {
    .PUERTO_ESCUCHA = "8000",
    .TAM_MEMORIA = 1024,
    .TAM_PAGINA = 64,
    .ENTRADAS_POR_TABLA = 4,
    .CANTIDAD_NIVELES = 2,
    .RETARDO_MEMORIA = 10,
    .RETARDO_SWAP = 20,
    .PATH_SWAPFILE = "/tmp/test_swap.bin",
    .PATH_INSTRUCCIONES = "/tmp",
    .LOG_LEVEL = LOG_LEVEL_INFO,
    .DUMP_PATH = "/tmp"
};

// Test básico para verificar inicialización y destrucción
void test_swap_init_destroy() {
    printf("Test: Inicialización y destrucción del sistema de swap\n");
    
    bool result = swap_manager_init(&test_config);
    if (!result) {
        printf("FALLÓ: No se pudo inicializar el swap manager\n");
        return;
    }
    
    printf("OK: Swap manager inicializado correctamente\n");
    
    t_swap_status status = swap_get_status();
    printf("- Total de páginas: %u\n", status.total_pages);
    printf("- Páginas usadas: %u\n", status.used_pages);
    printf("- Páginas libres: %zu\n", swap_get_free_pages_count());
    
    swap_manager_destroy();
    printf("OK: Swap manager destruido correctamente\n");
}

// Test para verificar la asignación de páginas
void test_swap_allocation() {
    printf("\nTest: Asignación de páginas en swap\n");
    
    if (!swap_manager_init(&test_config)) {
        printf("FALLÓ: No se pudo inicializar el swap manager\n");
        return;
    }
    
    // Asignar páginas para un proceso
    uint32_t pid1 = 100;
    uint32_t num_pages1 = 5;
    
    t_list* pages1 = swap_allocate_pages(pid1, num_pages1);
    if (!pages1) {
        printf("FALLÓ: No se pudieron asignar páginas para PID %u\n", pid1);
        swap_manager_destroy();
        return;
    }
    
    printf("OK: Se asignaron %u páginas para PID %u\n", num_pages1, pid1);
    
    // Asignar páginas para otro proceso
    uint32_t pid2 = 200;
    uint32_t num_pages2 = 7;
    
    t_list* pages2 = swap_allocate_pages(pid2, num_pages2);
    if (!pages2) {
        printf("FALLÓ: No se pudieron asignar páginas para PID %u\n", pid2);
        list_destroy_and_destroy_elements(pages1, free_swap_page_info);
        swap_manager_destroy();
        return;
    }
    
    printf("OK: Se asignaron %u páginas para PID %u\n", num_pages2, pid2);
    
    // Verificar estadísticas
    t_swap_status status = swap_get_status();
    printf("- Total de páginas: %u\n", status.total_pages);
    printf("- Páginas usadas: %u\n", status.used_pages);
    printf("- Páginas libres: %zu\n", swap_get_free_pages_count());
    
    // Liberar páginas del primer proceso
    swap_free_pages(pid1);
    printf("OK: Se liberaron las páginas del PID %u\n", pid1);
    
    status = swap_get_status();
    printf("- Páginas usadas después de liberar PID %u: %u\n", pid1, status.used_pages);
    
    // Liberar páginas del segundo proceso
    swap_free_pages(pid2);
    printf("OK: Se liberaron las páginas del PID %u\n", pid2);
    
    status = swap_get_status();
    printf("- Páginas usadas después de liberar PID %u: %u\n", pid2, status.used_pages);
    
    // Limpiar
    list_destroy_and_destroy_elements(pages1, free_swap_page_info);
    list_destroy_and_destroy_elements(pages2, free_swap_page_info);
    swap_manager_destroy();
}

// Test para verificar las operaciones de lectura y escritura
void test_swap_read_write() {
    printf("\nTest: Operaciones de lectura y escritura\n");
    
    if (!swap_manager_init(&test_config)) {
        printf("FALLÓ: No se pudo inicializar el swap manager\n");
        return;
    }
    
    // Asignar páginas para un proceso
    uint32_t pid = 300;
    uint32_t num_pages = 3;
    
    t_list* pages = swap_allocate_pages(pid, num_pages);
    if (!pages) {
        printf("FALLÓ: No se pudieron asignar páginas para PID %u\n", pid);
        swap_manager_destroy();
        return;
    }
    
    // Crear buffer con datos de prueba
    char* test_data = malloc(num_pages * test_config.TAM_PAGINA);
    if (!test_data) {
        printf("FALLÓ: No se pudo asignar memoria para datos de prueba\n");
        list_destroy_and_destroy_elements(pages, free_swap_page_info);
        swap_manager_destroy();
        return;
    }
    
    // Llenar con patrones reconocibles
    for (uint32_t i = 0; i < num_pages * test_config.TAM_PAGINA; i++) {
        test_data[i] = (i % 256);
    }
    
    // Escribir en swap
    if (!swap_write_pages(pages, test_data, test_config.TAM_PAGINA)) {
        printf("FALLÓ: Error escribiendo páginas en swap\n");
        free(test_data);
        list_destroy_and_destroy_elements(pages, free_swap_page_info);
        swap_manager_destroy();
        return;
    }
    
    printf("OK: Datos escritos en swap\n");
    
    // Limpiar buffer
    memset(test_data, 0, num_pages * test_config.TAM_PAGINA);
    
    // Leer desde swap
    if (!swap_read_pages(pages, test_data, test_config.TAM_PAGINA)) {
        printf("FALLÓ: Error leyendo páginas desde swap\n");
        free(test_data);
        list_destroy_and_destroy_elements(pages, free_swap_page_info);
        swap_manager_destroy();
        return;
    }
    
    printf("OK: Datos leídos desde swap\n");
    
    // Verificar datos
    bool data_valid = true;
    for (uint32_t i = 0; i < num_pages * test_config.TAM_PAGINA && data_valid; i++) {
        if ((unsigned char)test_data[i] != (i % 256)) {
            printf("FALLÓ: Datos incorrectos en posición %u (esperado %u, leído %u)\n", 
                   i, (i % 256), (unsigned char)test_data[i]);
            data_valid = false;
        }
    }
    
    if (data_valid) {
        printf("OK: Los datos leídos coinciden con los escritos\n");
    }
    
    // Verificar estadísticas
    t_swap_status status = swap_get_status();
    printf("- Operaciones de lectura: %u\n", status.read_operations);
    printf("- Operaciones de escritura: %u\n", status.write_operations);
    
    // Limpiar
    free(test_data);
    list_destroy_and_destroy_elements(pages, free_swap_page_info);
    swap_free_pages(pid);
    swap_manager_destroy();
}

// Estructura para pruebas de concurrencia
typedef struct {
    uint32_t pid;
    uint32_t num_pages;
    int iterations;
    bool success;
} test_thread_params;

// Función para hilo de prueba
void* test_thread_function(void* arg) {
    test_thread_params* params = (test_thread_params*)arg;
    params->success = true;
    
    for (int i = 0; i < params->iterations && params->success; i++) {
        // Asignar páginas
        t_list* pages = swap_allocate_pages(params->pid, params->num_pages);
        if (!pages) {
            printf("Hilo PID %u: FALLÓ asignación de páginas\n", params->pid);
            params->success = false;
            break;
        }
        
        // Crear buffer con datos
        char* data = malloc(params->num_pages * test_config.TAM_PAGINA);
        if (!data) {
            printf("Hilo PID %u: FALLÓ asignación de buffer\n", params->pid);
            list_destroy_and_destroy_elements(pages, free_swap_page_info);
            params->success = false;
            break;
        }
        
        // Escribir datos específicos del PID
        for (uint32_t j = 0; j < params->num_pages * test_config.TAM_PAGINA; j++) {
            data[j] = (params->pid + j) % 256;
        }
        
        // Escribir en swap
        if (!swap_write_pages(pages, data, test_config.TAM_PAGINA)) {
            printf("Hilo PID %u: FALLÓ escritura en swap\n", params->pid);
            free(data);
            list_destroy_and_destroy_elements(pages, free_swap_page_info);
            params->success = false;
            break;
        }
        
        // Limpiar buffer
        memset(data, 0, params->num_pages * test_config.TAM_PAGINA);
        
        // Leer desde swap
        if (!swap_read_pages(pages, data, test_config.TAM_PAGINA)) {
            printf("Hilo PID %u: FALLÓ lectura desde swap\n", params->pid);
            free(data);
            list_destroy_and_destroy_elements(pages, free_swap_page_info);
            params->success = false;
            break;
        }
        
        // Verificar datos
        for (uint32_t j = 0; j < params->num_pages * test_config.TAM_PAGINA; j++) {
            if ((unsigned char)data[j] != ((params->pid + j) % 256)) {
                printf("Hilo PID %u: FALLÓ verificación de datos\n", params->pid);
                params->success = false;
                break;
            }
        }
        
        // Limpiar para esta iteración
        free(data);
        swap_free_pages(params->pid);
        list_destroy_and_destroy_elements(pages, free_swap_page_info);
    }
    
    return NULL;
}

// Test para verificar concurrencia
void test_swap_concurrency() {
    printf("\nTest: Acceso concurrente al swap\n");
    
    if (!swap_manager_init(&test_config)) {
        printf("FALLÓ: No se pudo inicializar el swap manager\n");
        return;
    }
    
    // Configurar hilos de prueba
    const int num_threads = 4;
    pthread_t threads[num_threads];
    test_thread_params params[num_threads];
    
    // Crear los hilos
    for (int i = 0; i < num_threads; i++) {
        params[i].pid = 1000 + i;
        params[i].num_pages = 2 + i;  // Cada hilo usa un número diferente de páginas
        params[i].iterations = 5;
        params[i].success = false;
        
        if (pthread_create(&threads[i], NULL, test_thread_function, &params[i]) != 0) {
            printf("FALLÓ: Error creando hilo %d\n", i);
            swap_manager_destroy();
            return;
        }
    }
    
    // Esperar a que terminen los hilos
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        printf("Hilo PID %u: %s\n", params[i].pid, params[i].success ? "OK" : "FALLÓ");
    }
    
    // Verificar estadísticas
    t_swap_status status = swap_get_status();
    printf("- Operaciones de lectura: %u\n", status.read_operations);
    printf("- Operaciones de escritura: %u\n", status.write_operations);
    
    swap_manager_destroy();
}

int main() {
    printf("=== PRUEBAS DEL MÓDULO SWAP ===\n\n");
    
    // Ejecutar pruebas
    test_swap_init_destroy();
    test_swap_allocation();
    test_swap_read_write();
    test_swap_concurrency();
    
    printf("\n=== PRUEBAS COMPLETADAS ===\n");
    return 0;
}
