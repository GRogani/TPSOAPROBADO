#ifndef KERNEL_CONFIG
#define KERNEL_CONFIG

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "utils/config/config.h"
#include <commons/log.h>
#include <enum/planification-algorithm.enum.h>


typedef struct {
	char* memory_ip;
    int memory_port;
    t_planification_algorithm planification_algorithm;
    int sleep_time;
    t_log_level log_level;
} t_kernel_config;

t_config* init_and_validate(t_kernel_config* kernel_config);


#endif