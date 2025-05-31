#ifndef UTILS_CONFIG_H
#define UTILS_CONFIG_H

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../../macros/logger_macro.h"
#include "t_configs.h"

t_config* init_config(char* configFileName);
t_kernel_config init_kernel_config(t_config* config);
t_memoria_config init_memoria_config(t_config* config);
t_cpu_config init_cpu_config(t_config* config);
t_io_config init_io_config(t_config* config);

#endif