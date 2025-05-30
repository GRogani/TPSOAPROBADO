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
#include "lists_and_mutex.h"



t_package* fetch(int socket,uint32_t PID, uint32_t PC);

t_instruction* decode(t_package* package);

int execute(t_instruction* instruction, t_package* instruction_package, int socket_memory, int socket_dispatch, uint32_t* PC);

void* interrupt_listener(void* socket);

void* interrupt_handler(void* thread_args) ;

int check_interrupt(int socket_interrupt,t_package* package, uint32_t* pid_on_execute, uint32_t* pc_on_execute);

uint32_t MMU(uint32_t logic_dir);

#endif