#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/swap_space/swap_manager.h"
#include "src/utils.h"

// Configuración de prueba
t_memoria_config test_config = {
    .TAM_PAGINA = 4096,
    .PATH_SWAPFILE = "test_swapfile.bin"
};

void test_swap_basic() {
    printf("=== Test Básico de Swap Manager ===\n");
    
    // Inicializar swap manager
    if (!swap_manager_init(&test_config)) {
        printf("ERROR: No se pudo inicializar el swap manager\n");
        return;
    }
    printf("✓ Swap manager inicializado correctamente\n");
    
    // Asignar páginas para un proceso
    uint32_t pid = 123;
    uint32_t num_pages = 5;
    
    t_list* pages = swap_allocate_pages(pid, num_pages);
    if (pages == NULL) {
        printf("ERROR: No se pudieron asignar páginas\n");
        swap_manager_destroy();
        return;
    }
    printf("✓ Se asignaron %d páginas para PID %u\n", list_size(pages), pid);
    
    // Crear datos de prueba
    char* test_data = calloc(num_pages, test_config.TAM_PAGINA);
    if (test_data == NULL) {
        printf("ERROR: No se pudo asignar memoria de prueba\n");
        list_destroy_and_destroy_elements(pages, free);
        list_destroy(pages);
        swap_manager_destroy();
        return;
    }
    
    // Llenar con datos de prueba
    for (uint32_t i = 0; i < num_pages; i++) {
        char* page_data = test_data + (i * test_config.TAM_PAGINA);
        sprintf(page_data, "Página %u del proceso %u", i, pid);
    }
    
    // Escribir páginas al swap
    if (!swap_write_pages(pages, test_data, test_config.TAM_PAGINA)) {
        printf("ERROR: No se pudieron escribir páginas al swap\n");
        free(test_data);
        list_destroy_and_destroy_elements(pages, free);
        list_destroy(pages);
        swap_manager_destroy();
        return;
    }
    printf("✓ Páginas escritas al swap correctamente\n");
    
    // Crear buffer para leer
    char* read_data = calloc(num_pages, test_config.TAM_PAGINA);
    if (read_data == NULL) {
        printf("ERROR: No se pudo asignar memoria para lectura\n");
        free(test_data);
        list_destroy_and_destroy_elements(pages, free);
        list_destroy(pages);
        swap_manager_destroy();
        return;
    }
    
    // Leer páginas del swap
    if (!swap_read_pages(pages, read_data, test_config.TAM_PAGINA)) {
        printf("ERROR: No se pudieron leer páginas del swap\n");
        free(read_data);
        free(test_data);
        list_destroy_and_destroy_elements(pages, free);
        list_destroy(pages);
        swap_manager_destroy();
        return;
    }
    printf("✓ Páginas leídas del swap correctamente\n");
    
    // Verificar que los datos son correctos
    bool data_correct = true;
    for (uint32_t i = 0; i < num_pages; i++) {
        char* original_page = test_data + (i * test_config.TAM_PAGINA);
        char* read_page = read_data + (i * test_config.TAM_PAGINA);
        if (strcmp(original_page, read_page) != 0) {
            printf("ERROR: Datos incorrectos en página %u\n", i);
            data_correct = false;
            break;
        }
    }
    
    if (data_correct) {
        printf("✓ Datos verificados correctamente\n");
    }
    
    // Liberar páginas del swap
    swap_free_pages(pid);
    printf("✓ Páginas liberadas del swap\n");
    
    // Limpiar memoria
    free(read_data);
    free(test_data);
    list_destroy_and_destroy_elements(pages, free);
    list_destroy(pages);
    
    // Destruir swap manager
    swap_manager_destroy();
    printf("✓ Swap manager destruido correctamente\n");
    
    printf("=== Test completado exitosamente ===\n\n");
}

void test_swap_multiple_processes() {
    printf("=== Test Múltiples Procesos ===\n");
    
    if (!swap_manager_init(&test_config)) {
        printf("ERROR: No se pudo inicializar el swap manager\n");
        return;
    }
    
    // Crear múltiples procesos
    uint32_t pids[] = {100, 200, 300};
    uint32_t num_pages[] = {3, 2, 4};
    t_list* pages_list[3];
    
    for (int i = 0; i < 3; i++) {
        pages_list[i] = swap_allocate_pages(pids[i], num_pages[i]);
        if (pages_list[i] == NULL) {
            printf("ERROR: No se pudieron asignar páginas para PID %u\n", pids[i]);
            // Limpiar procesos anteriores
            for (int j = 0; j < i; j++) {
                swap_free_pages(pids[j]);
                list_destroy_and_destroy_elements(pages_list[j], free);
                list_destroy(pages_list[j]);
            }
            swap_manager_destroy();
            return;
        }
        printf("✓ PID %u: %u páginas asignadas\n", pids[i], num_pages[i]);
    }
    
    // Liberar procesos en orden inverso
    for (int i = 2; i >= 0; i--) {
        swap_free_pages(pids[i]);
        list_destroy_and_destroy_elements(pages_list[i], free);
        list_destroy(pages_list[i]);
        printf("✓ PID %u: páginas liberadas\n", pids[i]);
    }
    
    swap_manager_destroy();
    printf("=== Test múltiples procesos completado ===\n\n");
}

void test_swap_edge_cases() {
    printf("=== Test Casos Extremos ===\n");
    
    if (!swap_manager_init(&test_config)) {
        printf("ERROR: No se pudo inicializar el swap manager\n");
        return;
    }
    
    // Test 1: 0 páginas
    t_list* pages = swap_allocate_pages(999, 0);
    if (pages != NULL) {
        printf("✓ Test 0 páginas: OK\n");
        list_destroy(pages);
    } else {
        printf("ERROR: Test 0 páginas falló\n");
    }
    
    // Test 2: PID 0
    pages = swap_allocate_pages(0, 1);
    if (pages != NULL) {
        printf("✓ Test PID 0: OK\n");
        swap_free_pages(0);
        list_destroy_and_destroy_elements(pages, free);
        list_destroy(pages);
    } else {
        printf("ERROR: Test PID 0 falló\n");
    }
    
    // Test 3: Muchas páginas
    pages = swap_allocate_pages(888, 100);
    if (pages != NULL) {
        printf("✓ Test 100 páginas: OK\n");
        swap_free_pages(888);
        list_destroy_and_destroy_elements(pages, free);
        list_destroy(pages);
    } else {
        printf("ERROR: Test 100 páginas falló\n");
    }
    
    swap_manager_destroy();
    printf("=== Test casos extremos completado ===\n\n");
}

int main() {
    printf("Iniciando tests del Swap Manager...\n\n");
    
    // Eliminar archivo de swap si existe
    unlink("test_swapfile.bin");
    
    test_swap_basic();
    test_swap_multiple_processes();
    test_swap_edge_cases();
    
    printf("Todos los tests completados.\n");
    return 0;
} 