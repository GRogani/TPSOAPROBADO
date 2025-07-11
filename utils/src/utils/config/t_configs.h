#ifndef UTILS_T_CONFIG_H
#define UTILS_T_CONFIG_H

#include "../../enums/Eplanification_algorithm.h"
#include <commons/log.h>
#include <stdint.h>

typedef struct t_kernel_config{
    char* memory_ip;
    char* memory_port;
    PLANIFICATION_ALGORITHM short_planification_algorithm;
    PLANIFICATION_ALGORITHM long_planification_algorithm;
    bool preemption_enabled;
    int sleep_time;
    double alpha;
    t_log_level log_level;
    char* cpu_dispatch_port;
    char* cpu_interrupt_port;
    char* io_port;
    int cpu_quantity;
    uint64_t default_estimated_cpu_burst_ms;
} t_kernel_config;

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

typedef struct t_cpu_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* IP_KERNEL;
    char* PUERTO_KERNEL_DISPATCH;
    char* PUERTO_KERNEL_INTERRUPT;
    int ENTRADAS_TLB;
    char* REEMPLAZO_TLB;
    int ENTRADAS_CACHE;
    char* REEMPLAZO_CACHE;
    int RETARDO_CACHE;
    t_log_level LOG_LEVEL;
} t_cpu_config;

typedef struct t_io_config 
{
    char* IP_KERNEL;
    char* PUERTO_KERNEL;
    t_log_level LOG_LEVEL;
} t_io_config;


#endif