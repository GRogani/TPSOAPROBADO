#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/swap_space/swap_manager.h"

// Configuración simplificada para pruebas
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

int main() {
    printf("=== TEST SWAP MANAGER ===\n");
    
    // Inicializar swap manager
    printf("Inicializando swap manager...\n");
    if (!swap_manager_init(&test_config)) {
        printf("ERROR: No se pudo inicializar swap manager\n");
        return 1;
    }
    printf("Swap manager inicializado correctamente\n");
    
    // Asignar páginas para un proceso
    uint32_t pid1 = 100;
    uint32_t num_pages1 = 5;
    
    printf("Asignando %u páginas para PID %u...\n", num_pages1, pid1);
    t_list* pages1 = swap_allocate_pages(pid1, num_pages1);
    if (!pages1) {
        printf("ERROR: No se pudieron asignar páginas\n");
        swap_manager_destroy();
        return 1;
    }
    printf("Se asignaron %d páginas para PID %u\n", list_size(pages1), pid1);
    
    // Escribir datos a swap
    char* test_data = calloc(num_pages1, test_config.TAM_PAGINA);
    if (!test_data) {
        printf("ERROR: No se pudo asignar memoria para datos de prueba\n");
        list_destroy_and_destroy_elements(pages1, free);
        swap_manager_destroy();
        return 1;
    }
    
    // Llenar con un patrón reconocible
    for (uint32_t i = 0; i < num_pages1 * test_config.TAM_PAGINA; i++) {
        test_data[i] = i % 256;
    }
    
    printf("Escribiendo datos a swap...\n");
    if (!swap_write_pages(pages1, test_data, test_config.TAM_PAGINA)) {
        printf("ERROR: No se pudieron escribir datos a swap\n");
        free(test_data);
        list_destroy_and_destroy_elements(pages1, free);
        swap_manager_destroy();
        return 1;
    }
    printf("Datos escritos correctamente\n");
    
    // Limpiar buffer
    memset(test_data, 0, num_pages1 * test_config.TAM_PAGINA);
    
    // Leer datos de swap
    printf("Leyendo datos de swap...\n");
    if (!swap_read_pages(pages1, test_data, test_config.TAM_PAGINA)) {
        printf("ERROR: No se pudieron leer datos de swap\n");
        free(test_data);
        list_destroy_and_destroy_elements(pages1, free);
        swap_manager_destroy();
        return 1;
    }
    printf("Datos leídos correctamente\n");
    
    // Verificar datos
    bool data_valid = true;
    for (uint32_t i = 0; i < num_pages1 * test_config.TAM_PAGINA && data_valid; i++) {
        if ((unsigned char)test_data[i] != (i % 256)) {
            printf("ERROR: Datos incorrectos en posición %u (esperado %u, leído %u)\n", 
                  i, (unsigned char)(i % 256), (unsigned char)test_data[i]);
            data_valid = false;
        }
    }
    
    if (data_valid) {
        printf("Verificación de datos: OK\n");
    } else {
        printf("Verificación de datos: FALLÓ\n");
    }
    
    // Liberar páginas
    printf("Liberando páginas para PID %u...\n", pid1);
    swap_free_pages(pid1);
    printf("Páginas liberadas correctamente\n");
    
    // Limpiar
    free(test_data);
    list_destroy_and_destroy_elements(pages1, free);
    
    // Destruir swap manager
    printf("Destruyendo swap manager...\n");
    swap_manager_destroy();
    printf("Swap manager destruido correctamente\n");
    
    printf("=== TEST COMPLETADO ===\n");
    return 0;
}
