#ifndef CPU_MAIN_H
#define CPU_MAIN_H

#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/string.h>
#include "../utils.h"
#include "lists_and_mutex.h"
#include "instructionCycle.h"
#include "config_loader.h"
#include "mmu.h"

void cleanup_instruction(t_instruction* instruction);

#endif