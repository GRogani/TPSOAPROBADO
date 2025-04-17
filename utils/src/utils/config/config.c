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