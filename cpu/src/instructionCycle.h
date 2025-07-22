#ifndef CPU_INSTRUCTIONCYCLE_H
#define CPU_INSTRUCTIONCYCLE_H

#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include "../utils.h"
#include "cpuProtocol.h"
#include "mmu.h"
#include "lists_and_mutex.h"
#include <pthread.h>

typedef struct t_instruction
{
  INSTRUCTION_CODE instruction_code;
  int32_t operand_numeric1;
  int32_t operand_numeric2;
  int32_t operand_string_size;
  char *operand_string;
} t_instruction;

t_package *fetch(int socket, int32_t PID, int32_t PC);

t_instruction *decode(t_package *package);

void parse_instruction(char *instruction_string, t_instruction *instruction);

bool execution(t_instruction *instruction, int socket_memory, int socket_dispatch, _Atomic int32_t *pid, _Atomic int32_t *PC);

void *interrupt_listener(void *socket);

bool interrupt_handler(void *thread_args);

void check_interrupt(int socket_interrupt, t_package *package,_Atomic int32_t *pid_on_execute,_Atomic int32_t *pc_on_execute, int socket_memory);

int32_t MMU(int32_t logic_dir);

#endif