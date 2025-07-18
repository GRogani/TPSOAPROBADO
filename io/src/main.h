#ifndef IO_MAIN_H
#define IO_MAIN_H

#include "../utils.h"

void waiting_requests(int kernel_socket, char* id_IO);
void processing_operation(io_operation_package_data* args);

#endif