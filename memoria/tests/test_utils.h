#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdint.h>
#include <commons/log.h>

// Estructura de configuración simplificada para el test
typedef struct t_memoria_config {
    char* PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_PAGINA;
    int ENTRADAS_POR_TABLA;
    int CANTIDAD_NIVELES;
    int RETARDO_MEMORIA;
    int RETARDO_SWAP;
    char* PATH_SWAPFILE;
    char* PATH_INSTRUCCIONES;
    t_log_level LOG_LEVEL;
    char* DUMP_PATH;
} t_memoria_config;

extern t_memoria_config memoria_config;

// Definición de la función safe_malloc usada en page_table.c
void *safe_malloc(size_t size);

#define INVALID_FRAME_NUMBER -1

#endif // TEST_UTILS_H
