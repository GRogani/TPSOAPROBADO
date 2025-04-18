#include "config.h"

t_config* init_config(char* configFileName)
{
	t_config* config;

	config = config_create(configFileName);

	if(config == NULL) {
		LOG_ERROR("No se puedo crear el config.");
		exit(EXIT_FAILURE);
	}

	return config;
}

t_kernel_config init_kernel_config(t_config* config) {
    t_kernel_config conf;
    int LOG_LEVEL_INVALID = -1;
    char* planification_algorithm_str = config_get_string_value(config, "PLANIFICATION_ALGORITHM");
    char* log_level_str = config_get_string_value(config, "LOG_LEVEL");

    conf.planification_algorithm = planification_from_string(planification_algorithm_str);
    conf.log_level = log_level_from_string(log_level_str);

    if (conf.planification_algorithm == PLANIFICATION_INVALID) {
        LOG_ERROR("Algoritmo de planificacion invalido: %s", planification_algorithm_str);
        exit(EXIT_FAILURE);
    }

    if (conf.log_level == LOG_LEVEL_INVALID) {
        LOG_ERROR("Nivel de log invalido: %s", log_level_str);
        exit(EXIT_FAILURE);
    }

    conf.memory_ip = config_get_string_value(config, "MEMORY_IP");
    conf.memory_port = config_get_string_value(config, "MEMORY_PORT");

    conf.sleep_time = config_get_int_value(config, "SLEEP_TIME");

    return conf;
}

t_memoria_config init_memoria_config(t_config* config) {
    t_memoria_config conf;

    conf.PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
    conf.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    conf.TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
    conf.ENTRADAS_POR_TABLA = config_get_int_value(config, "ENTRADAS_POR_TABLA");
    conf.CANTIDAD_NIVELES = config_get_int_value(config, "CANTIDAD_NIVELES");
    conf.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    conf.RETARDO_SWAP = config_get_int_value(config, "RETARDO_SWAP");
    conf.PATH_SWAPFILE = config_get_string_value(config, "PATH_SWAPFILE");
    conf.LOG_LEVEL = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));
    conf.DUMP_PATH = config_get_string_value(config, "DUMP_PATH");

    return conf;
}

t_cpu_config init_cpu_config(t_config* config) {
    t_cpu_config conf;

    conf.IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    conf.PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
    conf.IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
    conf.PUERTO_KERNEL_DISPATCH = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
    conf.PUERTO_KERNEL_INTERRUPT = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
    conf.ENTRADAS_TLB = config_get_int_value(config, "ENTRADAS_TLB");
    conf.REEMPLAZO_TLB = config_get_string_value(config, "REEMPLAZO_TLB");
    conf.ENTRADAS_CACHE = config_get_int_value(config, "ENTRADAS_CACHE");
    conf.REEMPLAZO_CACHE = config_get_string_value(config, "REEMPLAZO_CACHE");
    conf.RETARDO_CACHE = config_get_int_value(config, "RETARDO_CACHE");
    conf.LOG_LEVEL = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));

    return conf;
}

t_io_config init_io_config(t_config* config) {
    t_io_config conf;

    conf.IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
    conf.PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
    conf.LOG_LEVEL = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));

    return conf;
}