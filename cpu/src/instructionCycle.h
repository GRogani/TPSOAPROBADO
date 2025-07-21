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
  uint32_t operand_numeric1;
  uint32_t operand_numeric2;
  uint32_t operand_string_size;
  char *operand_string;
} t_instruction;

t_package *fetch(int socket, uint32_t PID, uint32_t PC);

t_instruction *decode(t_package *package);

void parse_instruction(char *instruction_string, t_instruction *instruction);

bool execution(t_instruction *instruction, int socket_memory, int socket_dispatch, uint32_t *pid, uint32_t *PC);

void *interrupt_listener(void *socket);

bool interrupt_handler(void *thread_args);

void check_interrupt(int socket_interrupt, t_package *package, uint32_t *pid_on_execute, uint32_t *pc_on_execute, int socket_memory);

uint32_t MMU(uint32_t logic_dir);

#endif