#include "config.h"

t_config* init_config(char* configFileName)
{
	t_config* config;

	config = config_create(configFileName);

	return config;
}