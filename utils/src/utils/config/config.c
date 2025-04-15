#include "config.h"

t_config* init_config(char* configFileName)
{
	t_config* config;

	config = config_create(configFileName);

	if(config == NULL) {
		// add log from macro
		exit(EXIT_FAILURE);
	}

	return config;
}