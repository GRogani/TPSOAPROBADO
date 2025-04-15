#ifndef UTILS_LOGGER
#define UTILS_LOGGER

#include<stdlib.h>
#include<commons/log.h>

t_log* init_logger(char* log_file_name, char* process_name, t_log_level log_level);

#endif