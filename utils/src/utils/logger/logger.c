#include "logger.h"

static t_log* logger_instance = NULL;

void init_logger(char* log_file_name, char* process_name, t_log_level log_level) {

    if (logger_instance != NULL)
    {
        LOG_WARN("Ya habia un logger inicializado");
        return;
    }
    logger_instance = log_create(log_file_name, process_name, true, log_level);

    if (logger_instance == NULL) 
        LOG_ERROR("Fallo al crear el logger\n");

    else if (LOG_DEBUG_MODE)
        LOG_DEBUG("Logger creado en %p\n", logger_instance);
    
    return;
}

t_log* get_logger()
{
	return logger_instance;
}

void destroy_logger(void) 
{
    if (logger_instance != NULL) {
        log_destroy(logger_instance);
        logger_instance = NULL;
    }
}