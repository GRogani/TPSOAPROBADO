#ifndef CPU_INSTRUCTIONCYCLE_H
#define CPU_INSTRUCTIONCYCLE_H

#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdlib.h>
#include <utils/logger/logger.h>
#include "cpuProtocol.h"

instruction_t* fetch(int socket, int PC);
int decode_execute(instruction_t* instruction);


#endif