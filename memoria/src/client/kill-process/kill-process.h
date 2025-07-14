#ifndef KILL_PROCESS_H
#define KILL_PROCESS_H

#include <stdint.h>
#include "../utils.h"
#include "../../kernel_space/process_manager.h"

void delete_process_request_handler(int socket, t_package *package);

#endif