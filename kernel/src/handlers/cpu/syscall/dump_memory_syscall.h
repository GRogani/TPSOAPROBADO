#ifndef KERNEL_DUMP_MEMORY_SYSCALL_H
#define KERNEL_DUMP_MEMORY_SYSCALL_H

#include "../utils.h"
#include "repository/process/exit_list.h"
#include "repository/process/blocked_list.h"
#include "repository/process/ready_list.h"
#include "kernel_logic/scheduler/long_scheduler.h"
#include "kernel_logic/scheduler/short_scheduler.h"
#include "repository/pcb/pcb.h"
#include "handlers/memory/client/memory_client.h"
#include "handlers/cpu/syscall/exit_syscall.h"

void dump_memory_syscall(int32_t pid, int32_t pc);


#endif