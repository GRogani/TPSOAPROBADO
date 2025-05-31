#include "logger.h"

//USAR LAS MACROS DE LOGGING DEFINIDAS EN logger_macro.h

static t_log* logger_instance = NULL;

static sem_t logger_mutex;

void init_logger(char* log_file_name, char* process_name, t_log_level log_level) {

    if (logger_instance != NULL)
    {
        printf("\x1b[33m[WARNING]:\tYa habia un logger inicializado\n\x1b[0m");
        return;
    }
    logger_instance = log_create(log_file_name, process_name, true, log_level);

    if (logger_instance == NULL) 
        printf("\x1b[31m[ERROR]:\tFallo al crear el logger\n\x1b[0m");
    
    
    sem_init(&logger_mutex, 0, 1);
    
}

void lock_logger()
{
    sem_wait(&logger_mutex);
}

void unlock_logger()
{
    sem_post(&logger_mutex);
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
    sem_destroy(&logger_mutex);
}