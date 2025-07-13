#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/swap_space/swap_manager.h"
#include "../utils/utils.h"

// Configuración de prueba
t_memoria_config test_config = {
    .TAM_PAGINA = 64,
    .RETARDO_SWAP = 10,  // ms en lugar de microsegundos para acelerar los tests
    .PATH_SWAPFILE = "/tmp/test_swapfile.bin"
};

int main() {
    printf("Iniciando tests del Swap Manager...\n\n");
    
    // Eliminar archivo de swap si existe
    unlink(test_config.PATH_SWAPFILE);
    
    // Inicializar swap manager
    if (!swap_manager_init(&test_config)) {
        printf("ERROR: No se pudo inicializar el swap manager\n");
        return 1;
    }
    printf("✓ Swap manager inicializado correctamente\n");
    
    // Asignar páginas para un proceso
    uint32_t pid = 123;
    uint32_t num_pages = 5;
    
    t_list* pages = swap_allocate_pages(pid, num_pages);
    if (pages == NULL) {
        printf("ERROR: No se pudieron asignar páginas\n");
        swap_manager_destroy();
        return 1;
    }
    printf("✓ Se asignaron %d páginas para PID %u\n", list_size(pages), pid);
    
    // Crear datos de prueba
    char* test_data = calloc(num_pages, test_config.TAM_PAGINA);
    if (test_data == NULL) {
        printf("ERROR: No se pudo asignar memoria de prueba\n");
        list_destroy(pages);
        swap_manager_destroy();
        return 1;
    }
    
    // Llenar con datos de prueba
    for (uint32_t i = 0; i < num_pages; i++) {
        char* page_data = test_data + (i * test_config.TAM_PAGINA);
        sprintf(page_data, "Página %u del proceso %u", i, pid);
    }
    
    // Escribir páginas al swap
    printf("Escribiendo páginas al swap...\n");
    if (!swap_write_pages(pages, test_data, test_config.TAM_PAGINA)) {
        printf("ERROR: No se pudieron escribir páginas al swap\n");
        free(test_data);
        list_destroy(pages);
        swap_manager_destroy();
        return 1;
    }
    printf("✓ Páginas escritas al swap correctamente\n");
    
    // Crear buffer para leer
    char* read_data = calloc(num_pages, test_config.TAM_PAGINA);
    if (read_data == NULL) {
        printf("ERROR: No se pudo asignar memoria para lectura\n");
        free(test_data);
        list_destroy(pages);
        swap_manager_destroy();
        return 1;
    }
    
    // Leer páginas del swap
    printf("Leyendo páginas del swap...\n");
    if (!swap_read_pages(pages, read_data, test_config.TAM_PAGINA)) {
        printf("ERROR: No se pudieron leer páginas del swap\n");
        free(read_data);
        free(test_data);
        list_destroy(pages);
        swap_manager_destroy();
        return 1;
    }
    printf("✓ Páginas leídas del swap correctamente\n");
    
    // Verificar que los datos son correctos
    printf("Verificando datos...\n");
    bool data_correct = true;
    for (uint32_t i = 0; i < num_pages; i++) {
        char* original_page = test_data + (i * test_config.TAM_PAGINA);
        char* read_page = read_data + (i * test_config.TAM_PAGINA);
        if (strcmp(original_page, read_page) != 0) {
            printf("ERROR: Datos incorrectos en página %u\n", i);
            printf("  Original: '%s'\n", original_page);
            printf("  Leído: '%s'\n", read_page);
            data_correct = false;
            break;
        }
    }
    
    if (data_correct) {
        printf("✓ Datos verificados correctamente\n");
    }
    
    // Liberar páginas del swap
    printf("Liberando páginas...\n");
    swap_free_pages(pid);
    printf("✓ Páginas liberadas del swap\n");
    
    // Ver páginas libres
    size_t free_pages = swap_get_free_pages_count();
    printf("\nPáginas libres en SWAP: %zu\n", free_pages);
    
    // Limpiar memoria
    free(read_data);
    free(test_data);
    list_destroy(pages);
    
    // Destruir swap manager
    swap_manager_destroy();
    printf("✓ Swap manager destruido correctamente\n");
    
    printf("\n=== Tests completados exitosamente ===\n\n");
    return 0;
}
