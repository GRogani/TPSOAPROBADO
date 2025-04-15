#include "config.h"

t_config* init_config_and_validate(t_kernel_config* kernel_config) {
    int not_valid = -1;

    t_config* config = init_config("kernel.config");

    int kernel_port = config_get_int_value(config, "PORT");
    char* memory_ip = config_get_string_value(config, "MEMORY_IP");
	int memory_port = config_get_int_value(config, "MEMORY_PORT");
	char* planification_algorithm_config = config_get_string_value(config, "PLANIFICATION_ALGORITHM");
    int sleep_time = config_get_int_value(config, "SLEEP_TIME");
    char* log_level_config = config_get_string_value(config, "LOG_LEVEL");

    t_log_level log_level = log_level_from_string(log_level_config);
    PLANIFICATION_ALGORITHM planification_algorithm = planification_from_string(planification_algorithm_config);

    if(log_level == not_valid) {
        fprintf(stderr, "Log level from config is not a valid enum value"); // TODO: use macro
        exit(EXIT_FAILURE);
    }

    if(planification_algorithm == not_valid) {
        fprintf(stderr, "Planitifcation algorithm from config is not a valid enum value"); // TODO: use macro
        exit(EXIT_FAILURE);
    }

    kernel_config->log_level = log_level;
    kernel_config->memory_port = string_itoa(memory_port);
    kernel_config->port = string_itoa(kernel_port);
    kernel_config->sleep_time = sleep_time;
    kernel_config->memory_ip = memory_ip;
    kernel_config->planification_algorithm = planification_algorithm;

    return config;
}
