#ifndef CPU_INSTRUCTIONCYCLE_H
#define CPU_INSTRUCTIONCYCLE_H

#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdlib.h>
#include <utils/logger/logger.h>
#include "cpuProtocol.h"

t_package* fetch(int socket, int PC);
instruction_t* decode(t_package* package);
int execute(instruction_t* instruction,t_package* instruction_package, int socket_memory, int* pc);
int check_interrupt(int socket_interrupt, int pid_on_execute, int pc_on_execute);

#endif