#include "logger.h"

t_log* init_logger(char* log_file_name, char* process_name, t_log_level log_level)
{
	t_log* logger;

	logger = log_create(log_file_name, process_name, true, log_level);

	return logger;
}