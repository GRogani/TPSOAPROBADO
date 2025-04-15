#include "config.h"

t_config* init_config_and_validate(t_kernel_config* kernel_config) {
    t_config* config = init_config("kernel.config");

    char* memory_ip = config_get_string_value(config, "MEMORY_IP");
	int memory_port = config_get_int_value(config, "MEMORY_PORT");
	char* planification_algorithm = config_get_string_value(config, "PLANIFICATION_ALGORITHM");
    int sleep_time = config_get_int_value(config, "SLEEP_TIME");
    char* log_level = config_get_string_value(config, "LOG_LEVEL");

    t_log_level log_level_enum = log_level_from_string(log_level);
    t_planification_algorithm planification_enum = planification_from_string(planification_algorithm);

    if(log_level_enum == -1) {
        printf("Log level from config is not a valid enum value");
        exit(EXIT_FAILURE);
    }

    if(planification_enum == -1) {
        printf("Planitifcation algorithm from config is not a valid enum value");
        exit(EXIT_FAILURE);
    }

    kernel_config->log_level = log_level_enum;
    kernel_config->memory_port = string_itoa(memory_port);
    kernel_config->sleep_time = sleep_time;
    kernel_config->memory_ip = memory_ip;
    kernel_config->planification_algorithm = planification_enum;

    return config;
}


void destroy_kernel_config(t_kernel_config* kernel_config) {
    free(kernel_config->memory_port);
    free(kernel_config);
}