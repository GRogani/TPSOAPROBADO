#ifndef KERNEL_EXIT_SYSCALL_H
#define KERNEL_EXIT_SYSCALL_H

#include "../utils.h"
#include "../../repository/process/exit_list.h"
#include "../scheduler/long_scheduler.h"
#include "../scheduler/short_scheduler.h"

void exit_process_syscall(uint32_t syscall);


#endif