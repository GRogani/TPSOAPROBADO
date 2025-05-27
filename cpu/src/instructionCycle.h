#ifndef CPU_INSTRUCTIONCYCLE_H
#define CPU_INSTRUCTIONCYCLE_H

#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include "../utils.h"
#include "cpuProtocol.h"



t_package* fetch(int socket,uint32_t PID, uint32_t PC);

instruction_t* decode(t_package* package);

int execute(instruction_t* instruction, t_package* instruction_package, int socket_memory, int socket_dispatch, uint32_t* PC);

int check_interrupt(int socket_interrupt, int pid_on_execute, int pc_on_execute);

void* interrupt_handler(void* thread_args) ;

uint32_t MMU(uint32_t logic_dir);

#endif