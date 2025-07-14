#ifndef FETCH_INSTRUCTION_H
#define FETCH_INSTRUCTION_H

#include <stdint.h>
#include "../utils.h"
#include "../../kernel_space/process_manager.h"

void get_instruction_request_handler(int socket, t_package *package);

#endif