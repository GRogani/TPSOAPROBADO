#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include<stdio.h>
#include<stdlib.h>
#include <string.h>

#include <commons/log.h>
#include <commons/string.h>

#include "utils/config/config.h"
#include "enum/Eplanification_algorithm.h"


// typedef struct t_kernel_config{
// 	char* port;
//     char* memory_ip;
//     char* memory_port;
//     PLANIFICATION_ALGORITHM planification_algorithm;
//     int sleep_time;
//     t_log_level log_level;
// } t_kernel_config;

t_config* init_config_and_validate(t_kernel_config* kernel_config);


#endif