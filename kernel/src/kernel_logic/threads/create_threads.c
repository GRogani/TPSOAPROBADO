#include "create_threads.h"

int create_io_server_thread(pthread_t* io_thread)
{

    // //CPU SERVER T
    // int err_cpu = pthread_create(cpu_thread, NULL, cpu_server_handler, NULL);
    // if (err_cpu != 0) 
    // {
    //     LOG_ERROR("Failed to create CPU server thread");
    //     return -1;
    // } else 
    // {
    //     LOG_DEBUG("Successfully created CPU server thread");
    // }

    //SERVER IO T
    int err_io = pthread_create(io_thread, NULL, io_server_handler, NULL);
    if (err_io != 0) 
    {
        LOG_ERROR("Failed to create IO server thread");
        return -1;
    } else 
    {
        LOG_INFO("Successfully created IO server thread");
    }

    return 0;
}