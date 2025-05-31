#include "io_completion.h"

void* io_completion(void *thread_args)
{
    t_completion_thread_args *args = (t_completion_thread_args *)thread_args;

    // get pid from blocked
    // -> move from BLOCKED to READY
    // -> mandamos a replanificar (corremos corto plazo)

    // if not found, get from susp blocked
    // -> move from SUSPENDED_BLOCKED to SUSPENDED_READY
    // -> mandamos a replanificar (corremos corto plazo)
    // deberiamos correr el largo plazo?? -> NO, no necesariamente se liberó la memoria principal.

    // if not found in any of the lists, log error.
    
    
    // actualizamos current-processing de la connection a -1.

    free(args->device_name);
    free(args);
}